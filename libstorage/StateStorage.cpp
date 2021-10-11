#include "StateStorage.h"
#include "../libutilities/Error.h"
#include <tbb/parallel_do.h>
#include <tbb/parallel_sort.h>
#include <tbb/spin_mutex.h>
#include <boost/lexical_cast.hpp>
#include <boost/throw_exception.hpp>
#include <optional>

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
        for (auto& entryIt : tableIt->second.entries)
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
                             StorageError::ReadError, "Get primary keys from prev failed!", *error),
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
        auto entryIt = tableIt->second.entries.find(_key);
        if (entryIt != tableIt->second.entries.end())
        {
            auto& entry = std::get<Entry>(entryIt->second);
            if (!entry.rollbacked() && entry.status() != Entry::DELETED)
            {
                _callback(nullptr, std::make_optional(std::get<Entry>(entryIt->second)));
                return;
            }
        }
    }

    if (m_prev)
    {
        m_prev->asyncGetRow(
            table, _key, [this, key = std::string(_key), _callback](auto&& error, auto&& entry) {
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
    Entry entry, std::function<void(Error::UniquePtr&&)> callback) noexcept
{
    auto setEntryToTable = [this, entry = std::move(entry)](const std::string_view& key,
                               std::vector<char> keyVec, TableData& table,
                               std::function<void(Error::UniquePtr &&)> callback) mutable {
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
                entryOld = std::make_optional(std::move(existsEntry));
            }
            std::get<Entry>(entryIt->second) = std::move(entry);

            keyView = entryIt->first;
        }
        else
        {
            keyView = std::string_view(keyVec.data(), keyVec.size());
            table.entries.emplace(keyView, std::make_tuple(std::move(keyVec), std::move(entry)));
        }

        if (m_recoder.local())
        {
            m_recoder.local()->log(Recoder::Change(
                table.tableInfo->name(), keyView, std::move(entryOld), table.dirty));
        }
        table.dirty = true;

        callback(nullptr);
    };

    auto tableIt = m_data.find(table);
    if (tableIt != m_data.end())
    {
        setEntryToTable(
            key, std::vector<char>(key.begin(), key.end()), tableIt->second, std::move(callback));
    }
    else
    {
        asyncOpenTable(table, [this, callback = std::move(callback),
                                  setEntryToTable = std::move(setEntryToTable),
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
                    callback(BCOS_ERROR_UNIQUE_PTR(
                        StorageError::WriteError, "Insert table: " + std::string(tableIt->first) +
                                                      " into tableFactory failed!"));
                    return;
                }

                std::string_view keyView(keyVec.data(), keyVec.size());
                setEntryToTable(keyView, std::move(keyVec), tableIt->second, std::move(callback));
                return;
            }
            else
            {
                callback(BCOS_ERROR_UNIQUE_PTR(
                    StorageError::TableNotExists, "Async set row failed, table does not exists"));
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
                auto& entry = std::get<1>(it.second);
                if (entry.rollbacked())
                {
                    continue;
                }
                bufferLength += entry.capacityOfHashField();
            }

            bcos::bytes buffer;
            buffer.reserve(bufferLength);
            for (auto& it : item.second.entries)
            {
                auto& entry = std::get<1>(it.second);
                if (entry.rollbacked())
                {
                    continue;
                }

                for (auto value : entry)
                {
                    buffer.insert(buffer.end(), value.begin(), value.end());
                }
            }

            auto hash = hashImpl->hash(buffer);

            tbb::spin_mutex::scoped_lock lock(mutex);
            totalHash ^= hash;
        }
    });


    return totalHash;
}

// std::vector<std::tuple<std::string, crypto::HashType StateStorage::tableHashes(
//     const bcos::crypto::Hash::Ptr& hashImpl)
// {
//     std::vector<std::tuple<std::string, crypto::HashType>> result;

//     for (auto& tableIt : m_data)
//     {
//         if (!tableIt.second.dirty)
//         {
//             continue;
//         }

//         result.push_back({std::string(tableIt.first), crypto::HashType()});
//     }

//     tbb::parallel_sort(result.begin(), result.end(),
//         [](const std::tuple<std::string, crypto::HashType>& lhs,
//             const std::tuple<std::string, crypto::HashType>& rhs) {
//             return std::get<0>(lhs) < std::get<0>(rhs);
//         });

//     tbb::parallel_for(
//         tbb::blocked_range<size_t>(0, result.size()), [&](const tbb::blocked_range<size_t>&
//         range) {
//             for (auto i = range.begin(); i != range.end(); ++i)
//             {
//                 auto& key = std::get<0>(result[i]);
//                 auto& table = m_data.find(key)->second.entries;

//                 std::vector<std::string> sortedEntries;
//                 size_t totalSize = 0;
//                 sortedEntries.reserve(table.size());
//                 for (auto& entryIt : table)
//                 {
//                     if (!std::get<Entry>(entryIt.second).rollbacked())
//                     {
//                         sortedEntries.push_back(std::string(entryIt.first));
//                         if (std::get<Entry>(entryIt.second).status() == Entry::DELETED)
//                         {
//                             totalSize += (entryIt.first.size() + 1);
//                         }
//                         else
//                         {
//                             totalSize +=
//                                 (entryIt.first.size() +
//                                     std::get<Entry>(entryIt.second).capacityOfHashField() + 1);
//                         }
//                     }
//                 }

//                 tbb::parallel_sort(sortedEntries.begin(), sortedEntries.end());

//                 bcos::bytes data(totalSize);
//                 size_t offset = 0;
//                 for (auto& key : sortedEntries)
//                 {
//                     memcpy(&(data.data()[offset]), key.data(), key.size());
//                     offset += key.size();

//                     auto& entry = std::get<Entry>(table.find(key)->second);
//                     if (entry.status() != Entry::DELETED)
//                     {
//                         for (auto field : entry)
//                         {
//                             memcpy(&(data.data()[offset]), field.data(), field.size());
//                             offset += field.size();
//                         }
//                     }

//                     data.data()[offset] = (char)entry.status();
//                     ++offset;
//                 }

//                 std::get<1>(result[i]) = hashImpl->hash(data);
//             }
//         });

//     return result;
// }

void StateStorage::rollback(const Recoder::Ptr& recoder)
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
                std::get<Entry>(tableMap[change.key]) = std::move(*(change.entry));
            }
            else
            {  // nullptr means the key is not exist in m_cache
                Entry oldEntry;
                oldEntry.setRollbacked(true);

                std::get<Entry>(tableMap[change.key]) = std::move(oldEntry);
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

    auto keyVec = std::vector<char>(key.begin(), key.end());
    auto keyView = std::string_view(keyVec.data(), keyVec.size());
    auto [it, success] = tableIt->second.entries.emplace(
        keyView, std::make_tuple(std::move(keyVec), std::move(entry)));

    if (!success)
    {
        BOOST_THROW_EXCEPTION(
            BCOS_ERROR(StorageError::WriteError, "Insert existing entry failed, entry exists"));
    }

    return std::get<1>(it->second);
}
