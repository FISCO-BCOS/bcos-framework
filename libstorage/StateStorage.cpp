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

    auto tableIt = m_tableInfos.find(table);
    if (tableIt != m_tableInfos.end())
    {
        for (auto& entryIt : m_data)
        {
            if (entryIt.first.table() == table &&
                (!_condition || _condition->isValid(entryIt.first.key())))
            {
                if (!entryIt.second.rollbacked())
                {
                    localKeys.insert({entryIt.first.key(), entryIt.second.status()});
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
    auto tableIt = m_tableInfos.find(table);
    if (tableIt != m_tableInfos.end())
    {
        auto entryIt = m_data.find(DataKey(table, _key));
        if (entryIt != m_data.end())
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
        [this, &table, &_callback](auto&& _keys) {
            std::vector<std::optional<Entry>> results(_keys.size());
            auto missinges = std::tuple<std::vector<std::string_view>,
                std::vector<std::tuple<std::string, size_t>>>();

            long existsCount = 0;

            auto tableIt = m_tableInfos.find(table);
            if (tableIt != m_tableInfos.end())
            {
                size_t i = 0;
                for (auto& key : _keys)
                {
                    auto entryIt = m_data.find(DataKey(table, key));
                    if (entryIt != m_data.end() && !entryIt->second.rollbacked())
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
                    [this, callback = std::move(_callback),
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
                _callback(nullptr, std::move(results));
            }
        },
        _keys);
}

void StateStorage::asyncSetRow(const std::string_view& table, const std::string_view& key,
    Entry entry, std::function<void(Error::UniquePtr)> callback)
{
    auto setEntryToTable = [this, entry = std::move(entry)](
                               std::variant<std::string_view, std::string> key,
                               TableInfo::ConstPtr tableInfo,
                               std::function<void(Error::UniquePtr &&)> callback) mutable {
        if (!entry.tableInfo())
        {
            entry.setTableInfo(tableInfo);
        }

        ssize_t updatedCapacity = entry.capacityOfHashField();
        std::optional<Entry> entryOld;
        std::string keyView;
        std::visit([&keyView](auto&& key) { keyView = std::string_view(key); }, key);

        auto entryIt = m_data.find(DataKey(tableInfo->name(), keyView));
        if (entryIt != m_data.end())
        {
            auto& existsEntry = entryIt->second;
            if (!existsEntry.rollbacked())
            {
                entryOld.emplace(std::move(existsEntry));
            }
            entryIt->second = std::move(entry);
            keyView = entryIt->first.key();

            updatedCapacity = updatedCapacity - entryOld->capacityOfHashField();
        }
        else
        {
            std::string keyString;
            std::visit([&keyString](auto&& key) { keyString = std::string(std::move(key)); }, key);
            auto it =
                m_data.emplace(DataKey(tableInfo->name(), std::move(keyString)), std::move(entry));
            keyView = it.first->first.key();
        }

        if (m_recoder.local())
        {
            // m_recoder.local()->log(Recoder::Change(tableInfo->name(), keyView,
            // std::move(entryOld), table.dirty));
            // FIXME: no table dirty here
            m_recoder.local()->log(
                Recoder::Change(tableInfo->name(), keyView, std::move(entryOld), false));
        }
        // table.dirty = true;

        m_capacity += updatedCapacity;
        callback(nullptr);
    };

    auto tableIt = m_tableInfos.find(table);
    if (tableIt != m_tableInfos.end())
    {
        setEntryToTable(key, tableIt->second, std::move(callback));
    }
    else
    {
        asyncOpenTable(
            table, [this, callback = std::move(callback),
                       setEntryToTable = std::move(setEntryToTable), key = std::string(key)](
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
                        m_tableInfos.emplace(table->tableInfo()->name(), table->tableInfo());

                    if (!inserted)
                    {
                        callback(BCOS_ERROR_UNIQUE_PTR(StorageError::WriteError,
                            "Insert table: " + std::string(tableIt->first) +
                                " into tableFactory failed!"));
                        return;
                    }

                    setEntryToTable(std::move(key), tableIt->second, std::move(callback));
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
    tbb::parallel_do(m_data.begin(), m_data.end(), [&](const std::pair<const DataKey, Entry>& it) {
        auto& entry = it.second;
        if (onlyDirty)
        {
            if (entry.dirty())
            {
                callback(it.first.table(), it.first.key(), entry);
            }
        }
        else
        {
            callback(it.first.table(), it.first.key(), entry);
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
        size_t bufferLength = 0;
        for (auto& it : range)
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
        for (auto& it : range)
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
    });


    return totalHash;
}

void StateStorage::rollback(const Recoder::ConstPtr& recoder)
{
    for (auto& change : *recoder)
    {
        // Public Table API cannot be used here because it will add another change log entry.

        // change->table->rollback(change);
        auto tableIt = m_tableInfos.find(change.table);
        if (tableIt != m_tableInfos.end())
        {
            if (change.entry)
            {
                auto entryIt = m_data.find(DataKey(tableIt->first, change.key));
                if (entryIt != m_data.end())
                {
                    entryIt->second = std::move(*(change.entry));
                }
                else
                {
                    m_data.emplace(DataKey(tableIt->first, std::string(change.key)),
                        std::move(*(change.entry)));
                }
            }
            else
            {  // nullopt means the key is not exist in m_cache
                Entry oldEntry;
                oldEntry.setRollbacked(true);

                auto entryIt = m_data.find(DataKey(tableIt->first, change.key));
                if (entryIt != m_data.end())
                {
                    entryIt->second = std::move(oldEntry);
                }
                else
                {
                    m_data.emplace(
                        DataKey(tableIt->first, std::string(change.key)), std::move(oldEntry));
                }
            }

            // tableIt->second.dirty = change.tableDirty; // FIXME: remove useless table dirty
        }
    }
}

Entry& StateStorage::importExistingEntry(const std::string_view& key, Entry entry)
{
    entry.setDirty(false);

    auto tableIt = m_tableInfos.find(entry.tableInfo()->name());
    if (tableIt == m_tableInfos.end())
    {
        m_data.emplace(entry.tableInfo()->name(), entry.tableInfo());
    }

    auto it = m_data.find(DataKey(entry.tableInfo()->name(), key));
    if (it != m_data.end())
    {
        if (!it->second.rollbacked())
        {
            BOOST_THROW_EXCEPTION(
                BCOS_ERROR(StorageError::WriteError, "Insert existing entry failed, entry exists"));
        }

        it->second = std::move(entry);
        return it->second;
    }
    auto [insertedIt, inserted] =
        m_data.emplace(DataKey(entry.tableInfo()->name(), std::string(key)), std::move(entry));
    boost::ignore_unused(inserted);

    return insertedIt->second;
}
