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
    std::function<void(std::optional<Error>&&, std::vector<std::string>&&)> _callback) noexcept
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

        _callback({}, std::move(resultKeys));
        return;
    }

    m_prev->asyncGetPrimaryKeys(table, _condition,
        [localKeys = std::move(localKeys), callback = std::move(_callback)](
            auto&& error, auto&& remoteKeys) mutable {
            if (error)
            {
                callback(BCOS_ERROR_WITH_PREV(-1, "Get primary keys from prev failed!", *error),
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

            callback({}, std::move(remoteKeys));
        });
}

void StateStorage::asyncGetRow(const std::string_view& table, const std::string_view& _key,
    std::function<void(std::optional<Error>&&, std::optional<Entry>&&)> _callback) noexcept
{
    auto tableIt = m_data.find(table);
    if (tableIt != m_data.end())
    {
        auto entryIt = std::get<TableData>(tableIt->second).entries.find(_key);
        if (entryIt != std::get<TableData>(tableIt->second).entries.end() &&
            !std::get<Entry>(entryIt->second).rollbacked())
        {
            _callback({}, Entry(std::get<Entry>(entryIt->second)));
            return;
        }
    }

    if (m_prev)
    {
        m_prev->asyncGetRow(table, _key,
            [this, table = std::string(table), key = std::string(_key), _callback](
                auto&& error, auto&& entry) {
                if (error)
                {
                    _callback(
                        BCOS_ERROR_WITH_PREV(-1, "Get row from tableStorage failed!", *error), {});
                    return;
                }

                if (entry)
                {
                    // If the entry exists, add it to the local cache, for version comparison
                    _callback({}, Entry(importExistingEntry(key, std::move(*entry))));
                }
                else
                {
                    _callback({}, {});
                }
            });
    }
    else
    {
        _callback({}, {});
    }
}

void StateStorage::asyncGetRows(const std::string_view& table,
    const std::variant<gsl::span<std::string_view const>, gsl::span<std::string const>>& _keys,
    std::function<void(std::optional<Error>&&, std::vector<std::optional<Entry>>&&)>
        _callback) noexcept
{
    if (_keys.index() == 0)
    {
        multiAsyncGetRows(table, std::get<0>(_keys), _callback);
    }
    else if (_keys.index() == 1)
    {
        multiAsyncGetRows(table, std::get<1>(_keys), _callback);
    }
}

void StateStorage::asyncSetRow(const std::string_view& table, const std::string_view& key,
    Entry entry, std::function<void(std::optional<Error>&&, bool)> callback) noexcept
{
    entry.setNum(m_blockNumber);

    auto setEntryToTable = [this, entry = std::move(entry), tableName = std::string(table),
                               callback](const std::string_view& key,
                               std::unique_ptr<std::string> keyPtr, TableData& table) mutable {
        if (!entry.tableInfo())
        {
            entry.setTableInfo(table.tableInfo);
        }
        std::optional<Entry> entryOld;
        auto entryIt = table.entries.find(key);
        if (entryIt != table.entries.end())
        {
            auto& existsEntry = std::get<Entry>(entryIt->second);
            if (!existsEntry.rollbacked())
            {
                if (m_checkVersion && (entry.version() - existsEntry.version() != 1))
                {
                    callback(
                        BCOS_ERROR(-1,
                            "Entry version: " + boost::lexical_cast<std::string>(entry.version()) +
                                " mismatch (current version + 1): " +
                                boost::lexical_cast<std::string>(existsEntry.version())),
                        false);
                    return;
                }
                entryOld = std::move(existsEntry);
            }
            std::get<Entry>(entryIt->second) = std::move(entry);
        }
        else
        {
            std::unique_ptr<std::string> keyToInsert;
            if (keyPtr)
            {
                table.entries.emplace(key, std::make_tuple(std::move(keyPtr), std::move(entry)));
            }
            else
            {
                auto keyToInsert = std::make_unique<std::string>(key);
                std::string_view keyView(*keyToInsert);
                table.entries.emplace(
                    keyView, std::make_tuple(std::move(keyToInsert), std::move(entry)));
            }
        }

        getChangeLog().push_back(
            Change(tableName, Change::Set, std::string(key), std::move(entryOld),
                table.dirty));  // TODO: fix null tableInfo ptr
        table.dirty = true;

        callback({}, true);
    };

    auto tableIt = m_data.find(table);
    if (tableIt != m_data.end())
    {
        setEntryToTable(key, nullptr, std::get<1>(tableIt->second));
    }
    else
    {
        asyncOpenTable(table, [this, callback, setEntryToTable = std::move(setEntryToTable),
                                  key = std::string(key),
                                  tableName = std::string(table)](std::optional<Error>&& error,
                                  std::optional<Table>&& table) mutable {
            if (error)
            {
                callback(BCOS_ERROR_WITH_PREV(-1, "Open table: " + tableName + " failed", *error),
                    false);
                return;
            }

            if (table)
            {
                auto tableNamePtr = std::make_unique<std::string>(std::move(tableName));
                std::string_view tableNameView(*tableNamePtr);
                auto [tableIt, inserted] = m_data.emplace(tableNameView,
                    std::make_tuple(std::move(tableNamePtr), TableData(table->tableInfo())));

                if (!inserted)
                {
                    callback(BCOS_ERROR(-1, "Insert table: " + std::string(tableIt->first) +
                                                " into tableFactory failed!"),
                        false);
                    return;
                }

                auto keyPtr = std::make_unique<std::string>(std::move(key));
                std::string_view keyView(*keyPtr);
                setEntryToTable(keyView, std::move(keyPtr), std::get<1>(tableIt->second));
                return;
            }
            else
            {
                callback(BCOS_ERROR(
                             -1, "Async set row failed, table: " + tableName + " does not exists"),
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
        [&](const std::pair<const std::string_view,
            std::tuple<std::unique_ptr<std::string>, TableData>>& it) {
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
    std::promise<std::tuple<std::optional<Error>, std::optional<Table>>> openPromise;
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
    std::promise<std::tuple<std::optional<Error>, bool>> createPromise;
    asyncCreateTable(_tableName, _valueFields, [&](std::optional<Error>&& error, bool success) {
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
                        for (auto& field : entry)
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

Entry& StateStorage::importExistingEntry(std::string key, Entry entry)
{
    entry.setVersion(0);
    entry.setDirty(false);

    bool inserted;
    auto tableIt = m_data.find(entry.tableInfo()->name());
    if (tableIt == m_data.end())
    {
        auto tableNamePtr = std::make_unique<std::string>(entry.tableInfo()->name());
        auto tableNameView = std::string_view(*tableNamePtr);
        std::tie(tableIt, inserted) = m_data.emplace(
            tableNameView, std::make_tuple(std::move(tableNamePtr), TableData(entry.tableInfo())));
    }

    auto keyPtr = std::make_unique<std::string>(std::move(key));
    auto keyView = std::string_view(*keyPtr);
    auto [it, success] =
        std::get<TableData>(tableIt->second)
            .entries.emplace(keyView, std::make_tuple(std::move(keyPtr), std::move(entry)));

    if (!success)
    {
        BOOST_THROW_EXCEPTION(BCOS_ERROR(-1, "Insert existing entry failed, entry exists"));
    }

    return std::get<1>(it->second);
}
