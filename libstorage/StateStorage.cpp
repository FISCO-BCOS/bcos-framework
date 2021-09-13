#include "StateStorage.h"
#include "../libutilities/Error.h"
#include <tbb/parallel_do.h>
#include <tbb/parallel_sort.h>
#include <boost/lexical_cast.hpp>
#include <boost/throw_exception.hpp>

using namespace bcos;
using namespace bcos::storage;

void StateStorage::asyncGetPrimaryKeys(const std::string_view& table,
    const std::optional<Condition const>& _condition,
    std::function<void(Error::UniquePtr&&, std::vector<std::string>&&)> _callback) noexcept
{
    std::map<std::string_view, storage::Entry::Status> localKeys;

    auto tableIt = m_data.find(table);
    if (tableIt != m_data.end())
    {
        for (auto& entryIt : std::get<TableData>(tableIt->second).entries)
        {
            if (!_condition || _condition->isValid(entryIt.first))
            {
                if (!std::get<Entry>(entryIt.second).rollbacked())
                {
                    localKeys.insert({entryIt.first, std::get<Entry>(entryIt.second).status()});
                }
            }
        }
    }

    if (!m_prev)
    {
        std::vector<std::string> resultKeys;
        for (auto& localIt : localKeys)
        {
            if (localIt.second != Entry::DELETED)
            {
                resultKeys.push_back(std::string(localIt.first));
            }
        }

        _callback(nullptr, std::move(resultKeys));
        return;
    }

    m_prev->asyncGetPrimaryKeys(table, _condition,
        [localKeys = std::move(localKeys), callback = std::move(_callback)](
            auto&& error, auto&& remoteKeys) mutable {
            if (error)
            {
                callback(BCOS_ERROR_WITH_PREV_UNIQUE_PTR(
                             -1, "Get primary keys from prev failed!", *error),
                    std::vector<std::string>());
                return;
            }

            for (auto it = remoteKeys.begin(); it != remoteKeys.end(); ++it)
            {
                auto localIt = localKeys.find(*it);
                if (localIt != localKeys.end())
                {
                    if (localIt->second == Entry::DELETED)
                    {
                        it = remoteKeys.erase(it);
                    }

                    localKeys.erase(localIt);
                }
            }

            for (auto& localIt : localKeys)
            {
                if (localIt.second != Entry::DELETED)
                {
                    remoteKeys.push_back(std::string(localIt.first));
                }
            }

            callback(nullptr, std::move(remoteKeys));
        });
}

void StateStorage::asyncGetRow(const std::string_view& table, const std::string_view& _key,
    std::function<void(Error::UniquePtr&&, std::optional<Entry>&&)> _callback) noexcept
{
    auto tableIt = m_data.find(table);
    if (tableIt != m_data.end())
    {
        auto entryIt = std::get<TableData>(tableIt->second).entries.find(_key);
        if (entryIt != std::get<TableData>(tableIt->second).entries.end() &&
            !std::get<Entry>(entryIt->second).rollbacked())
        {
            _callback(nullptr, std::make_optional(std::get<Entry>(entryIt->second)));
            return;
        }
    }

    if (m_prev)
    {
        m_prev->asyncGetRow(
            table, _key, [this, key = std::string(_key), _callback](auto&& error, auto&& entry) {
                if (error)
                {
                    _callback(
                        BCOS_ERROR_WITH_PREV_UNIQUE_PTR(-1, "Get row from storage failed!", *error),
                        {});
                    return;
                }

                if (entry)
                {
                    // If the entry exists, add it to the local cache, for version comparison
                    _callback(
                        nullptr, std::make_optional(importExistingEntry(key, std::move(*entry))));
                }
                else
                {
                    _callback(nullptr, {});
                }
            });
    }
    else
    {
        _callback({}, {});
    }
}

void StateStorage::asyncGetRows(const std::string_view& table,
    const std::variant<const gsl::span<std::string_view const>, const gsl::span<std::string const>>&
        _keys,
    std::function<void(Error::UniquePtr&&, std::vector<std::optional<Entry>>&&)> _callback) noexcept
{
    if (_keys.index() == 0)
    {
        multiAsyncGetRows(table, std::get<0>(_keys), std::move(_callback));
    }
    else if (_keys.index() == 1)
    {
        multiAsyncGetRows(table, std::get<1>(_keys), std::move(_callback));
    }
}

void StateStorage::asyncSetRow(const std::string_view& table, const std::string_view& key,
    Entry entry, std::function<void(Error::UniquePtr&&, bool)> callback) noexcept
{
    entry.setNum(m_blockNumber);

    auto setEntryToTable = [this, entry = std::move(entry)](const std::string_view& key,
                               std::vector<char> keyVec, TableData& table,
                               std::function<void(Error::UniquePtr&&, bool)> callback) mutable {
        if (!entry.tableInfo())
        {
            entry.setTableInfo(table.tableInfo);
        }

        std::optional<Entry> entryOld;
        std::string_view keyView;
        auto entryIt = table.entries.find(key);
        if (entryIt != table.entries.end())
        {
            auto& existsEntry = std::get<Entry>(entryIt->second);
            if (!existsEntry.rollbacked())
            {
                if (m_checkVersion && (entry.version() - existsEntry.version() != 1))
                {
                    callback(
                        BCOS_ERROR_UNIQUE_PTR(-1,
                            "Entry version: " + boost::lexical_cast<std::string>(entry.version()) +
                                " mismatch (current version + 1): " +
                                boost::lexical_cast<std::string>(existsEntry.version())),
                        false);
                    return;
                }
                entryOld = std::move(existsEntry);
            }
            std::get<Entry>(entryIt->second) = std::move(entry);

            keyView = entryIt->first;
        }
        else
        {
            keyView = std::string_view(keyVec.data(), keyVec.size());
            table.entries.emplace(keyView, std::make_tuple(std::move(keyVec), std::move(entry)));
        }

        getChangeLog().emplace_back(
            table.tableInfo->name(), Change::Set, keyView, std::move(entryOld), table.dirty);
        table.dirty = true;

        callback(nullptr, true);
    };

    auto tableIt = m_data.find(table);
    if (tableIt != m_data.end())
    {
        setEntryToTable(key, std::vector<char>(key.begin(), key.end()),
            std::get<1>(tableIt->second), std::move(callback));
    }
    else
    {
        asyncOpenTable(table, [this, callback = std::move(callback),
                                  setEntryToTable = std::move(setEntryToTable),
                                  keyVec = std::vector<char>(key.begin(), key.end())](
                                  Error::UniquePtr&& error, std::optional<Table>&& table) mutable {
            if (error)
            {
                callback(BCOS_ERROR_WITH_PREV_UNIQUE_PTR(-1, "Open table failed", *error), false);
                return;
            }

            if (table)
            {
                auto tableName = table->tableInfo()->name();
                std::vector<char> tableNameVec(tableName.begin(), tableName.end());
                std::string_view tableNameView(tableNameVec.data(), tableNameVec.size());
                auto [tableIt, inserted] = m_data.emplace(tableNameView,
                    std::make_tuple(std::move(tableNameVec), TableData(table->tableInfo())));

                if (!inserted)
                {
                    callback(
                        BCOS_ERROR_UNIQUE_PTR(-1, "Insert table: " + std::string(tableIt->first) +
                                                      " into tableFactory failed!"),
                        false);
                    return;
                }

                std::string_view keyView(keyVec.data(), keyVec.size());
                setEntryToTable(
                    keyView, std::move(keyVec), std::get<1>(tableIt->second), std::move(callback));
                return;
            }
            else
            {
                callback(BCOS_ERROR_UNIQUE_PTR(-1, "Async set row failed, table does not exists"),
                    false);
            }
        });
    }
}

void StateStorage::parallelTraverse(bool onlyDirty,
    std::function<bool(
        const std::string_view& table, const std::string_view& key, const Entry& entry)>
        callback) const
{
    tbb::parallel_do(m_data.begin(), m_data.end(),
        [&](const std::pair<const std::string_view, std::tuple<std::vector<char>, TableData>>& it) {
            for (auto& entryIt : std::get<TableData>(it.second).entries)
            {
                auto entry = std::get<Entry>(entryIt.second);
                if (onlyDirty)
                {
                    if (entry.dirty())
                    {
                        callback(it.first, entryIt.first, std::get<Entry>(entryIt.second));
                    }
                }
                else
                {
                    callback(it.first, entryIt.first, std::get<Entry>(entryIt.second));
                }
            }
        });
}

std::optional<Table> StateStorage::openTable(const std::string& tableName)
{
    std::promise<std::tuple<Error::UniquePtr, std::optional<Table>>> openPromise;
    asyncOpenTable(tableName, [&](auto&& error, auto&& table) {
        openPromise.set_value({std::move(error), std::move(table)});
    });

    auto [error, table] = openPromise.get_future().get();
    if (error)
    {
        BOOST_THROW_EXCEPTION(
            BCOS_ERROR_WITH_PREV(-1, "Open table: " + tableName + " failed", *error));
    }

    return table;
}

bool StateStorage::createTable(const std::string& _tableName, const std::string& _valueFields)
{
    std::promise<std::tuple<Error::UniquePtr, bool>> createPromise;
    asyncCreateTable(_tableName, _valueFields, [&](Error::UniquePtr&& error, bool success) {
        createPromise.set_value({std::move(error), success});
    });
    auto [error, success] = createPromise.get_future().get();
    if (error)
    {
        BOOST_THROW_EXCEPTION(BCOS_ERROR_WITH_PREV(-1,
            "Create table: " + _tableName + " valueFields: " + _valueFields + " failed", *error));
    }

    return success;
}

std::vector<std::tuple<std::string, crypto::HashType>> StateStorage::tableHashes()
{
    std::vector<std::tuple<std::string, crypto::HashType>> result;

    for (auto& tableIt : m_data)
    {
        if (!std::get<TableData>(tableIt.second).dirty)
        {
            continue;
        }

        result.push_back({std::string(tableIt.first), crypto::HashType()});
    }

    tbb::parallel_sort(result.begin(), result.end(),
        [](const std::tuple<std::string, crypto::HashType>& lhs,
            const std::tuple<std::string, crypto::HashType>& rhs) {
            return std::get<0>(lhs) < std::get<0>(rhs);
        });

    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, result.size()), [&](const tbb::blocked_range<size_t>& range) {
            for (auto i = range.begin(); i != range.end(); ++i)
            {
                auto& key = std::get<0>(result[i]);
                auto& table = std::get<TableData>(m_data.find(key)->second).entries;

                std::vector<std::string> sortedEntries;
                size_t totalSize = 0;
                sortedEntries.reserve(table.size());
                for (auto& entryIt : table)
                {
                    if (!std::get<Entry>(entryIt.second).rollbacked())
                    {
                        sortedEntries.push_back(std::string(entryIt.first));
                        if (std::get<Entry>(entryIt.second).status() == Entry::DELETED)
                        {
                            totalSize += (entryIt.first.size() + 1);
                        }
                        else
                        {
                            totalSize +=
                                (entryIt.first.size() +
                                    std::get<Entry>(entryIt.second).capacityOfHashField() + 1);
                        }
                    }
                }

                tbb::parallel_sort(sortedEntries.begin(), sortedEntries.end());

                bcos::bytes data(totalSize);
                size_t offset = 0;
                for (auto& key : sortedEntries)
                {
                    memcpy(&(data.data()[offset]), key.data(), key.size());
                    offset += key.size();

                    auto& entry = std::get<Entry>(table.find(key)->second);
                    if (entry.status() != Entry::DELETED)
                    {
                        for (auto field : entry)
                        {
                            memcpy(&(data.data()[offset]), field.data(), field.size());
                            offset += field.size();
                        }
                    }

                    data.data()[offset] = (char)entry.status();
                    ++offset;
                }

                std::get<1>(result[i]) = m_hashImpl->hash(data);
            }
        });

    return result;
}

void StateStorage::rollback(size_t _savepoint)
{
    auto& changeLog = getChangeLog();
    while (_savepoint < changeLog.size())
    {
        auto& change = changeLog.back();
        // Public Table API cannot be used here because it will add another change log entry.

        // change->table->rollback(change);
        auto tableIt = m_data.find(change.table);
        if (tableIt != m_data.end())
        {
            auto& tableMap = std::get<TableData>(tableIt->second).entries;
            switch (change.kind)
            {
            case Change::Set:
            {
                if (change.entry)
                {
                    std::get<Entry>(tableMap[change.key]) = std::move(*(change.entry));
                }
                else
                {  // nullptr means the key is not exist in m_cache
                    auto oldEntry = Entry(nullptr, m_blockNumber);
                    oldEntry.setRollbacked(true);
                    std::get<Entry>(tableMap[change.key]) = std::move(oldEntry);
                }

                break;
            }
            case Change::Remove:
            {
                std::get<Entry>(tableMap[change.key]).setStatus(storage::Entry::Status::NORMAL);
                if (change.entry)
                {
                    std::get<Entry>(tableMap[change.key]) = std::move(*(change.entry));
                }
                else
                {
                    std::get<Entry>(tableMap[change.key]).setRollbacked(true);
                }
                break;
            }
            default:
                break;
            }
        }

        changeLog.pop_back();
    }
}

Entry& StateStorage::importExistingEntry(const std::string_view& key, Entry entry)
{
    entry.setVersion(0);
    entry.setDirty(false);

    bool inserted;
    auto tableIt = m_data.find(entry.tableInfo()->name());
    if (tableIt == m_data.end())
    {
        auto tableName = entry.tableInfo()->name();
        auto tableNameVec = std::vector<char>(tableName.begin(), tableName.end());
        auto tableNameView = std::string_view(tableNameVec.data(), tableNameVec.size());
        std::tie(tableIt, inserted) = m_data.emplace(
            tableNameView, std::make_tuple(std::move(tableNameVec), TableData(entry.tableInfo())));
    }

    auto keyVec = std::vector<char>(key.begin(), key.end());
    auto keyView = std::string_view(keyVec.data(), keyVec.size());
    auto [it, success] =
        std::get<TableData>(tableIt->second)
            .entries.emplace(keyView, std::make_tuple(std::move(keyVec), std::move(entry)));

    if (!success)
    {
        BOOST_THROW_EXCEPTION(BCOS_ERROR(-1, "Insert existing entry failed, entry exists"));
    }

    return std::get<1>(it->second);
}
