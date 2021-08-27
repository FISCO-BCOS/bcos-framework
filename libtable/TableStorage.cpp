#include "TableStorage.h"
#include "../libutilities/Error.h"
#include <tbb/parallel_do.h>
#include <tbb/parallel_sort.h>
#include <boost/lexical_cast.hpp>

using namespace bcos;
using namespace bcos::storage;

void TableStorage::asyncGetPrimaryKeys(const bcos::storage::TableInfo::Ptr& _tableInfo,
    const bcos::storage::Condition::Ptr& _condition,
    std::function<void(bcos::Error::Ptr&&, std::vector<std::string>&&)> _callback) noexcept
{
    std::map<std::string, storage::Entry::Status> localKeys;

    auto tableIt = m_data.find(_tableInfo->name);
    if (tableIt != m_data.end())
    {
        for (auto& entryIt : tableIt->second.entries)
        {
            if (!_condition || _condition->isValid(entryIt.first))
            {
                if (!entryIt.second->rollbacked())
                {
                    localKeys.insert({entryIt.first, entryIt.second->status()});
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
                resultKeys.push_back(localIt.first);
            }
        }

        _callback(nullptr, std::move(resultKeys));
        return;
    }

    m_prev->asyncGetPrimaryKeys(_tableInfo, _condition,
        [localKeys = std::move(localKeys), _callback](
            Error::Ptr&& error, std::vector<std::string>&& remoteKeys) mutable {
            if (error)
            {
                _callback(BCOS_ERROR_WITH_PREV_PTR(-1, "Get primary keys from prev failed!", error),
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
                    remoteKeys.push_back(localIt.first);
                }
            }

            _callback(nullptr, std::move(remoteKeys));
        });
}

void TableStorage::asyncGetRow(const bcos::storage::TableInfo::Ptr& _tableInfo,
    const std::string& _key,
    std::function<void(bcos::Error::Ptr&&, bcos::storage::Entry::Ptr&&)> _callback) noexcept
{
    auto tableIt = m_data.find(_tableInfo->name);
    if (tableIt != m_data.end())
    {
        auto entryIt = tableIt->second.entries.find(_key);
        if (entryIt != tableIt->second.entries.end() && !entryIt->second->rollbacked())
        {
            _callback(nullptr, std::make_shared<Entry>(*(entryIt->second)));
            return;
        }
    }

    if (m_prev)
    {
        m_prev->asyncGetRow(
            _tableInfo, _key, [this, _tableInfo, _key, _callback](Error::Ptr&& error, Entry::Ptr&& entry) {
                if (error)
                {
                    _callback(
                        BCOS_ERROR_WITH_PREV_PTR(-1, "Get row from tableStorage failed!", error),
                        nullptr);
                    return;
                }

                // If the entry exists, add it to the local cache, for version comparison
                if (entry)
                {
                    entry->setVersion(0);
                    entry->setDirty(false);

                    auto [tableIt, inserted] =
                        m_data.insert({entry->tableInfo()->name, TableData()});
                    (void)inserted;
                    tableIt->second.tableInfo = _tableInfo;
                    tableIt->second.entries.insert({_key, entry});

                    _callback(nullptr, std::make_shared<Entry>(*entry));
                }
                else
                {
                    _callback(nullptr, nullptr);
                }
            });
    }
    else
    {
        _callback(nullptr, nullptr);
    }
}

void TableStorage::asyncGetRows(const bcos::storage::TableInfo::Ptr& _tableInfo,
    const gsl::span<std::string>& _keys,
    std::function<void(bcos::Error::Ptr&&, std::vector<bcos::storage::Entry::Ptr>&&)>
        _callback) noexcept
{
    auto results = std::make_shared<std::vector<storage::Entry::Ptr>>(_keys.size());
    auto missings = std::make_shared<std::tuple<std::vector<std::string>, std::vector<size_t>>>();

    long existsCount = 0;

    auto tableIt = m_data.find(_tableInfo->name);
    if (tableIt != m_data.end())
    {
        size_t i = 0;
        for (auto& key : _keys)
        {
            auto entryIt = tableIt->second.entries.find(key);
            if (entryIt != tableIt->second.entries.end())
            {
                (*results)[i] = std::make_shared<Entry>(*(entryIt->second));
                ++existsCount;
            }
            else
            {
                std::get<0>(*missings).push_back(key);
                std::get<1>(*missings).push_back(i);
            }

            ++i;
        }
    }

    if (existsCount < _keys.size() && m_prev)
    {
        m_prev->asyncGetRows(_tableInfo, std::get<0>(*missings),
            [_callback, missings, results](
                Error::Ptr&& error, std::vector<storage::Entry::Ptr>&& entries) {
                if (error)
                {
                    _callback(BCOS_ERROR_WITH_PREV_PTR(-1, "async get perv rows failed!", error),
                        std::vector<bcos::storage::Entry::Ptr>());
                    return;
                }

                for (size_t i = 0; i < entries.size(); ++i)
                {
                    (*results)[std::get<1>(*missings)[i]] = std::move(entries[i]);
                }

                _callback(nullptr, std::move(*results));
            });
    }
    else
    {
        _callback(nullptr, std::move(*results));
    }
}

void TableStorage::asyncSetRow(const bcos::storage::TableInfo::Ptr& tableInfo,
    const std::string& key, const bcos::storage::Entry::Ptr& entry,
    std::function<void(bcos::Error::Ptr&&, bool)> callback) noexcept
{
    auto entryCopy = std::make_shared<Entry>(*entry);

    auto tableIt = m_data.find(tableInfo->name);
    if (tableIt != m_data.end())
    {
        Entry::Ptr entryOld;
        auto entryIt = tableIt->second.entries.find(key);
        if (entryIt != tableIt->second.entries.end())
        {
            if (!entryIt->second->rollbacked())
            {
                if (entryCopy->version() - entryIt->second->version() != 1)
                {
                    callback(BCOS_ERROR_PTR(-1,
                                 "Entry version: " +
                                     boost::lexical_cast<std::string>(entryCopy->version()) +
                                     " mismatch (current version + 1): " +
                                     boost::lexical_cast<std::string>(entryIt->second->version())),
                        false);
                    return;
                }
                entryOld = std::move(entryIt->second);
            }
            entryIt->second = std::move(entryCopy);
        }
        else
        {
            tableIt->second.entries.insert({key, std::move(entryCopy)});
        }

        getChangeLog().push_back(
            Change(tableInfo, Change::Set, key, entryOld, tableIt->second.dirty));
        tableIt->second.dirty = true;

        callback(nullptr, true);
    }
    else
    {
        asyncOpenTable(tableInfo->name, [this, tableInfo, key, entryCopy = std::move(entryCopy),
                                            callback](
                                            Error::Ptr&& error, storage::Table::Ptr&& table) {
            if (error)
            {
                callback(BCOS_ERROR_WITH_PREV_PTR(
                             -1, "Open table: " + tableInfo->name + " failed", error),
                    false);
                return;
            }

            if (table)
            {
                auto [tableIt, inserted] = m_data.insert({tableInfo->name,
                    {tableInfo, tbb::concurrent_unordered_map<std::string, storage::Entry::Ptr>(),
                        false}});

                if (!inserted)
                {
                    callback(BCOS_ERROR_PTR(-1,
                                 "Insert table: " + tableInfo->name + " into tableFactory failed!"),
                        false);
                    return;
                }

                Entry::Ptr entryOld;
                auto entryIt = tableIt->second.entries.find(key);
                if (entryIt != tableIt->second.entries.end())
                {
                    if (!entryIt->second->rollbacked())
                    {
                        if (entryCopy->version() - entryIt->second->version() != 1)
                        {
                            callback(BCOS_ERROR_PTR(-1, "Entry version: " +
                                                            boost::lexical_cast<std::string>(
                                                                entryCopy->version()) +
                                                            " mismatch (current version + 1): " +
                                                            boost::lexical_cast<std::string>(
                                                                entryIt->second->version())),
                                false);
                            return;
                        }
                        entryOld = std::move(entryIt->second);
                    }
                    entryIt->second = std::move(entryCopy);
                }
                else
                {
                    tableIt->second.entries.insert({key, std::move(entryCopy)});
                }

                getChangeLog().push_back(
                    Change(tableInfo, Change::Set, key, entryOld, tableIt->second.dirty));
                tableIt->second.dirty = true;

                callback(nullptr, true);
            }
            else
            {
                callback(BCOS_ERROR_PTR(-1, "Async set row failed, table: " + tableInfo->name +
                                                " does not exists"),
                    false);
            }
        });
    }
}

void TableStorage::parallelTraverse(bool onlyDirty,
    std::function<bool(
        const TableInfo::Ptr& tableInfo, const std::string& key, const Entry::ConstPtr& entry)>
        callback) const
{
    tbb::parallel_do(
        m_data.begin(), m_data.end(), [&](const std::pair<std::string, TableData>& it) {
            for (auto& entryIt : it.second.entries)
            {
                if (onlyDirty)
                {
                    if (entryIt.second->dirty())
                    {
                        callback(it.second.tableInfo, entryIt.first, entryIt.second);
                    }
                }
                else
                {
                    callback(it.second.tableInfo, entryIt.first, entryIt.second);
                }
            }
        });
}

void TableStorage::asyncOpenTable(const std::string& tableName,
    std::function<void(Error::Ptr&&, storage::Table::Ptr&&)> callback) noexcept
{
    auto sysTableInfo = getSysTableInfo(tableName);
    if (sysTableInfo)
    {
        auto table =
            std::make_shared<storage::Table>(shared_from_this(), sysTableInfo, m_blockNumber);
        callback(nullptr, std::move(table));

        return;
    }
    else
    {
        asyncOpenTable(SYS_TABLES, [this, callback, tableName](
                                       Error::Ptr&& error, storage::Table::Ptr&& table) {
            if (error)
            {
                callback(std::move(error), nullptr);
                return;
            }

            table->asyncGetRow(tableName, [this, tableName, callback](
                                              Error::Ptr&& error, storage::Entry::Ptr&& entry) {
                if (error)
                {
                    callback(std::move(error), nullptr);
                    return;
                }

                if (!entry)
                {
                    callback(nullptr, nullptr);
                    return;
                }

                auto tableInfo = std::make_shared<storage::TableInfo>(tableName,
                    entry->getField(SYS_TABLE_KEY_FIELDS), entry->getField(SYS_TABLE_VALUE_FIELDS));
                auto table =
                    std::make_shared<storage::Table>(shared_from_this(), tableInfo, m_blockNumber);

                callback(nullptr, std::move(table));
            });
        });
    }
}

void TableStorage::asyncCreateTable(const std::string& _tableName, const std::string& _keyField,
    const std::string& _valueFields, std::function<void(Error::Ptr&&, bool)> callback) noexcept
{
    asyncOpenTable(SYS_TABLES, [_tableName, callback, _keyField, _valueFields](
                                   Error::Ptr&& error, storage::Table::Ptr&& sysTable) {
        if (error)
        {
            callback(BCOS_ERROR_WITH_PREV_PTR(-1, "Open sys_tables failed!", error), false);
            return;
        }

        sysTable->asyncGetRow(_tableName, [_tableName, callback, sysTable, _keyField, _valueFields](
                                              Error::Ptr&& error, storage::Entry::Ptr&& entry) {
            if (error)
            {
                callback(BCOS_ERROR_WITH_PREV_PTR(-1, "Get table info row failed!", error), false);
                return;
            }

            if (entry)
            {
                callback(nullptr, false);
                return;
            }

            auto tableEntry = sysTable->newEntry();
            tableEntry->setField(SYS_TABLE_KEY_FIELDS, _keyField);
            tableEntry->setField(SYS_TABLE_VALUE_FIELDS, _valueFields);

            sysTable->asyncSetRow(_tableName, tableEntry, [callback](Error::Ptr&& error, bool) {
                if (error)
                {
                    callback(BCOS_ERROR_WITH_PREV_PTR(
                                 -1, "Put table info into sys_tables failed!", error),
                        false);
                    return;
                }

                callback(nullptr, true);
            });
        });
    });
}

Table::Ptr TableStorage::openTable(const std::string& tableName)
{
    std::promise<std::tuple<Error::Ptr, Table::Ptr>> openPromise;
    asyncOpenTable(tableName, [&](Error::Ptr&& error, Table::Ptr&& table) {
        openPromise.set_value({std::move(error), std::move(table)});
    });

    auto [error, table] = openPromise.get_future().get();
    if (error)
    {
        BOOST_THROW_EXCEPTION(
            *(BCOS_ERROR_WITH_PREV_PTR(-1, "Open table: " + tableName + " failed", error)));
    }

    return table;
}

bool TableStorage::createTable(
    const std::string& _tableName, const std::string& _keyField, const std::string& _valueFields)
{
    std::promise<std::tuple<Error::Ptr, bool>> createPromise;
    asyncCreateTable(_tableName, _keyField, _valueFields, [&](Error::Ptr&& error, bool success) {
        createPromise.set_value({std::move(error), success});
    });
    auto [error, success] = createPromise.get_future().get();
    if (error)
    {
        BOOST_THROW_EXCEPTION(*(BCOS_ERROR_WITH_PREV_PTR(-1,
            "Create table: " + _tableName + " with keyField: " + _keyField +
                " valueFields: " + _valueFields + " failed",
            error)));
    }

    return success;
}

std::vector<std::tuple<std::string, crypto::HashType>> TableStorage::tablesHash()
{
    std::vector<std::tuple<std::string, crypto::HashType>> result;

    for (auto& tableIt : m_data)
    {
        if (!tableIt.second.dirty)
        {
            continue;
        }

        result.push_back({tableIt.first, crypto::HashType()});
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
                auto& table = m_data.find(key)->second.entries;

                std::vector<std::string> sortedEntries;
                size_t totalSize = 0;
                sortedEntries.reserve(table.size());
                for (auto& entryIt : table)
                {
                    if (!entryIt.second->rollbacked())
                    {
                        sortedEntries.push_back(entryIt.first);
                        if (entryIt.second->status() == Entry::DELETED)
                        {
                            totalSize += (entryIt.first.size() + 1);
                        }
                        else
                        {
                            totalSize +=
                                (entryIt.first.size() + entryIt.second->capacityOfHashField() + 1);
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

                    auto& entry = table.find(key)->second;
                    if (entry->status() != Entry::DELETED)
                    {
                        for (auto& field : *(entry))
                        {
                            memcpy(&(data.data()[offset]), field.data(), field.size());
                            offset += field.size();
                        }
                    }

                    data.data()[offset] = (char)entry->status();
                    ++offset;
                }

                std::get<1>(result[i]) = m_hashImpl->hash(data);
            }
        });

    return result;
}

void TableStorage::rollback(size_t _savepoint)
{
    auto& changeLog = getChangeLog();
    while (_savepoint < changeLog.size())
    {
        auto& change = changeLog.back();
        // Public Table API cannot be used here because it will add another change log entry.

        // change->table->rollback(change);
        auto tableIt = m_data.find(change.tableInfo->name);
        if (tableIt != m_data.end())
        {
            auto& tableMap = tableIt->second.entries;
            switch (change.kind)
            {
            case Change::Set:
            {
                if (change.entry)
                {
                    tableMap[change.key] = change.entry;
                }
                else
                {  // nullptr means the key is not exist in m_cache
                    auto oldEntry =
                        std::make_shared<storage::Entry>(change.tableInfo, m_blockNumber);
                    oldEntry->setRollbacked(true);
                    tableMap[change.key] = std::move(oldEntry);
                }

                break;
            }
            case Change::Remove:
            {
                tableMap[change.key]->setStatus(storage::Entry::Status::NORMAL);
                if (change.entry)
                {
                    tableMap[change.key] = change.entry;
                }
                else
                {
                    tableMap[change.key]->setRollbacked(true);
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
