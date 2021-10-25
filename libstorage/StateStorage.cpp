#include "StateStorage.h"
#include "../libutilities/Error.h"
#include <tbb/parallel_do.h>
#include <tbb/parallel_sort.h>
#include <tbb/spin_mutex.h>
#include <boost/core/ignore_unused.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/throw_exception.hpp>
#include <optional>

using namespace bcos;
using namespace bcos::storage;

void StateStorage::asyncGetPrimaryKeys(const std::string_view& table,
    const std::optional<Condition const>& _condition,
    std::function<void(Error::UniquePtr, std::vector<std::string>)> _callback)
{
    std::map<std::string_view, storage::Entry::Status> localKeys;

    auto tableIt = m_data.find(table);
    if (tableIt != m_data.end())
    {
        for (auto& entryIt : tableIt->second.entries)
        {
            if (!_condition || _condition->isValid(entryIt.first))
            {
                if (!entryIt.second.rollbacked())
                {
                    localKeys.insert({entryIt.first, entryIt.second.status()});
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
                             StorageError::ReadError, "Get primary keys from prev failed!", *error),
                    std::vector<std::string>());
                return;
            }

            for (auto it = remoteKeys.begin(); it != remoteKeys.end();)
            {
                bool deleted = false;

                auto localIt = localKeys.find(*it);
                if (localIt != localKeys.end())
                {
                    if (localIt->second == Entry::DELETED)
                    {
                        it = remoteKeys.erase(it);
                        deleted = true;
                    }

                    localKeys.erase(localIt);
                }

                if (!deleted)
                {
                    ++it;
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
    std::function<void(Error::UniquePtr, std::optional<Entry>)> _callback)
{
    auto tableIt = m_data.find(table);
    if (tableIt != m_data.end())
    {
        auto entryIt = tableIt->second.entries.find(_key);
        if (entryIt != tableIt->second.entries.end())
        {
            auto& entry = entryIt->second;

            if (!entry.rollbacked())
            {
                if (entry.status() == Entry::DELETED)
                {
                    _callback(nullptr, std::nullopt);
                }
                else
                {
                    _callback(nullptr, std::make_optional(entry));
                }
                return;
            }
        }
    }

    if (m_prev)
    {
        m_prev->asyncGetRow(table, _key,
            [this, key = std::string(_key), _callback](
                Error::UniquePtr error, std::optional<Entry> entry) {
                if (error)
                {
                    _callback(BCOS_ERROR_WITH_PREV_UNIQUE_PTR(
                                  StorageError::ReadError, "Get row from storage failed!", *error),
                        {});
                    return;
                }

                if (entry)
                {
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
    std::function<void(Error::UniquePtr, std::vector<std::optional<Entry>>)> _callback)
{
    std::visit(
        [this, table, callback = std::move(_callback)](auto&& _keys) {
            std::vector<std::optional<Entry>> results(_keys.size());
            auto missinges = std::tuple<std::vector<std::string_view>,
                std::vector<std::tuple<std::string, size_t>>>();

            long existsCount = 0;

            auto tableIt = m_data.find(table);
            if (tableIt != m_data.end())
            {
                size_t i = 0;
                for (auto& key : _keys)
                {
                    auto entryIt = tableIt->second.entries.find(key);
                    if (entryIt != tableIt->second.entries.end() && !entryIt->second.rollbacked())
                    {
                        Entry& entry = entryIt->second;
                        if (entry.status() != Entry::DELETED)
                        {
                            results[i].emplace(entry);
                        }
                        else
                        {
                            results[i] = std::nullopt;
                        }
                        ++existsCount;
                    }
                    else
                    {
                        std::get<1>(missinges).emplace_back(std::string(key), i);
                        std::get<0>(missinges).emplace_back(key);
                    }

                    ++i;
                }
            }
            else
            {
                for (long i = 0; i < _keys.size(); ++i)
                {
                    std::get<1>(missinges).emplace_back(std::string(_keys[i]), i);
                    std::get<0>(missinges).emplace_back(_keys[i]);
                }
            }

            if (existsCount < _keys.size() && m_prev)
            {
                m_prev->asyncGetRows(table, std::get<0>(missinges),
                    [this, callback = std::move(callback),
                        missingIndexes = std::move(std::get<1>(missinges)),
                        results = std::move(results)](
                        auto&& error, std::vector<std::optional<Entry>>&& entries) mutable {
                        if (error)
                        {
                            callback(BCOS_ERROR_WITH_PREV_UNIQUE_PTR(StorageError::ReadError,
                                         "async get perv rows failed!", *error),
                                std::vector<std::optional<Entry>>());
                            return;
                        }

                        for (size_t i = 0; i < entries.size(); ++i)
                        {
                            auto& entry = entries[i];

                            if (entry)
                            {
                                results[std::get<1>(missingIndexes[i])].emplace(importExistingEntry(
                                    std::move(std::get<0>(missingIndexes[i])), std::move(*entry)));
                            }
                        }

                        callback(nullptr, std::move(results));
                    });
            }
            else
            {
                callback(nullptr, std::move(results));
            }
        },
        _keys);
}

void StateStorage::asyncSetRow(const std::string_view& table, const std::string_view& key,
    Entry entry, std::function<void(Error::UniquePtr)> callback)
{
    auto setEntryToTable = [this, entry = std::move(entry)](const std::string_view& key,
                               TableData& table,
                               std::function<void(Error::UniquePtr &&)> callback) mutable {
        if (!entry.tableInfo())
        {
            entry.setTableInfo(table.tableInfo);
        }

        ssize_t updatedCapacity = entry.capacityOfHashField();
        std::optional<Entry> entryOld;
        std::string_view keyView;
        auto entryIt = table.entries.lower_bound(key);
        if (entryIt != table.entries.end() && entryIt->first == key)
        {
            auto& existsEntry = entryIt->second;
            if (!existsEntry.rollbacked())
            {
                entryOld.emplace(std::move(existsEntry));
            }
            entryIt->second = std::move(entry);

            updatedCapacity = updatedCapacity - entryOld->capacityOfHashField();
            keyView = entryIt->first;
        }
        else
        {
            auto it = table.entries.emplace_hint(entryIt, std::string(key), std::move(entry));
            keyView = it->first;
        }

        if (m_recoder.local())
        {
            m_recoder.local()->log(Recoder::Change(
                table.tableInfo->name(), keyView, std::move(entryOld), table.dirty));
        }
        table.dirty = true;

        m_capacity += updatedCapacity;
        callback(nullptr);
    };

    auto tableIt = m_data.find(table);
    if (tableIt != m_data.end())
    {
        setEntryToTable(key, tableIt->second, std::move(callback));
    }
    else
    {
        asyncOpenTable(table,
            [this, callback = std::move(callback), setEntryToTable = std::move(setEntryToTable),
                keyVec = std::vector<char>(key.begin(), key.end())](
                Error::UniquePtr&& error, std::optional<Table>&& table) mutable {
                if (error)
                {
                    callback(BCOS_ERROR_WITH_PREV_UNIQUE_PTR(
                        StorageError::ReadError, "Open table failed", *error));
                    return;
                }

                if (table)
                {
                    auto [tableIt, inserted] =
                        m_data.emplace(table->tableInfo()->name(), TableData(table->tableInfo()));

                    if (!inserted)
                    {
                        callback(BCOS_ERROR_UNIQUE_PTR(StorageError::WriteError,
                            "Insert table: " + std::string(tableIt->first) +
                                " into tableFactory failed!"));
                        return;
                    }

                    std::string_view keyView(keyVec.data(), keyVec.size());
                    setEntryToTable(keyView, tableIt->second, std::move(callback));
                    return;
                }
                else
                {
                    callback(BCOS_ERROR_UNIQUE_PTR(StorageError::TableNotExists,
                        "Async set row failed, table does not exists"));
                }
            });
    }
}

void StateStorage::parallelTraverse(bool onlyDirty,
    std::function<bool(
        const std::string_view& table, const std::string_view& key, const Entry& entry)>
        callback) const
{
    tbb::parallel_do(
        m_data.begin(), m_data.end(), [&](const std::pair<const std::string_view, TableData>& it) {
            for (auto& entryIt : it.second.entries)
            {
                auto entry = entryIt.second;
                if (onlyDirty)
                {
                    if (entry.dirty())
                    {
                        callback(it.first, entryIt.first, entryIt.second);
                    }
                }
                else
                {
                    callback(it.first, entryIt.first, entryIt.second);
                }
            }
        });
}

std::optional<Table> StateStorage::openTable(const std::string_view& tableName)
{
    std::promise<std::tuple<Error::UniquePtr, std::optional<Table>>> openPromise;
    asyncOpenTable(tableName, [&](auto&& error, auto&& table) {
        openPromise.set_value({std::move(error), std::move(table)});
    });

    auto [error, table] = openPromise.get_future().get();
    if (error)
    {
        BOOST_THROW_EXCEPTION(*error);
    }

    return table;
}

std::optional<Table> StateStorage::createTable(std::string _tableName, std::string _valueFields)
{
    std::promise<std::tuple<Error::UniquePtr, std::optional<Table>>> createPromise;
    asyncCreateTable(
        _tableName, _valueFields, [&](Error::UniquePtr&& error, std::optional<Table>&& table) {
            createPromise.set_value({std::move(error), std::move(table)});
        });
    auto [error, table] = createPromise.get_future().get();
    if (error)
    {
        BOOST_THROW_EXCEPTION(*error);
    }

    return table;
}

crypto::HashType StateStorage::hash(const bcos::crypto::Hash::Ptr& hashImpl)
{
    bcos::h256 totalHash;

    tbb::spin_mutex mutex;
    tbb::parallel_for(m_data.range(), [&totalHash, &mutex, &hashImpl](auto&& range) {
        for (auto& item : range)
        {
            size_t bufferLength = 0;
            for (auto& it : item.second.entries)
            {
                auto& entry = it.second;
                if (entry.rollbacked())
                {
                    continue;
                }
                bufferLength += (entry.capacityOfHashField() + 1);
            }

            bcos::bytes buffer;
            buffer.reserve(bufferLength);
            for (auto& it : item.second.entries)
            {
                auto& entry = it.second;
                if (entry.rollbacked())
                {
                    continue;
                }

                for (auto value : entry)
                {
                    buffer.insert(buffer.end(), value.begin(), value.end());
                }
                buffer.insert(buffer.end(), (char)entry.status());
            }

            auto hash = hashImpl->hash(buffer);

            tbb::spin_mutex::scoped_lock lock(mutex);
            totalHash ^= hash;
        }
    });


    return totalHash;
}

void StateStorage::rollback(const Recoder::ConstPtr& recoder)
{
    for (auto& change : *recoder)
    {
        // Public Table API cannot be used here because it will add another change log entry.

        // change->table->rollback(change);
        auto tableIt = m_data.find(change.table);
        if (tableIt != m_data.end())
        {
            auto& tableMap = tableIt->second.entries;
            if (change.entry)
            {
                auto entryIt = tableMap.lower_bound(change.key);
                if (entryIt != tableMap.end() && entryIt->first == entryIt->first)
                {
                    entryIt->second = std::move(*(change.entry));
                }
                else
                {
                    tableMap.emplace_hint(
                        entryIt, std::string(change.key), std::move(*(change.entry)));
                }
            }
            else
            {  // nullopt means the key is not exist in m_cache
                Entry oldEntry;
                oldEntry.setRollbacked(true);

                auto entryIt = tableMap.lower_bound(change.key);
                if (entryIt != tableMap.end() && entryIt->first == entryIt->first)
                {
                    entryIt->second = std::move(oldEntry);
                }
                else
                {
                    tableMap.emplace_hint(entryIt, std::string(change.key), std::move(oldEntry));
                }
            }

            tableIt->second.dirty = change.tableDirty;
        }
    }
}

Entry& StateStorage::importExistingEntry(const std::string_view& key, Entry entry)
{
    entry.setDirty(false);

    bool inserted;
    auto tableIt = m_data.find(entry.tableInfo()->name());
    if (tableIt == m_data.end())
    {
        std::tie(tableIt, inserted) =
            m_data.emplace(entry.tableInfo()->name(), TableData(entry.tableInfo()));
    }

    auto it = tableIt->second.entries.lower_bound(key);
    if (it != tableIt->second.entries.end() && it->first == key)
    {
        if (!it->second.rollbacked())
        {
            BOOST_THROW_EXCEPTION(
                BCOS_ERROR(StorageError::WriteError, "Insert existing entry failed, entry exists"));
        }

        it->second = std::move(entry);
        return it->second;
    }

    auto insertedIt = tableIt->second.entries.emplace_hint(it, std::string(key), std::move(entry));
    return insertedIt->second;
}
