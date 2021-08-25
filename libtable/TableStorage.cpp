#include "TableStorage.h"
#include "../libutilities/Error.h"

using namespace bcos;
using namespace bcos::stroage;

void TableStorage::asyncGetPrimaryKeys(const bcos::storage::TableInfo::Ptr& _tableInfo,
    const bcos::storage::Condition::Ptr& _condition,
    std::function<void(bcos::Error::Ptr&&, std::vector<std::string>&&)> _callback) noexcept
{}

void TableStorage::asyncGetRow(const bcos::storage::TableInfo::Ptr& _tableInfo,
    const std::string& _key,
    std::function<void(bcos::Error::Ptr&&, bcos::storage::Entry::Ptr&&)> _callback) noexcept
{
    auto tableIt = m_data.find(_tableInfo->name);
    if (tableIt != m_data.end())
    {
        auto entryIt = tableIt->second.entries.find(_key);
        if (entryIt != tableIt->second.entries.end())
        {
            auto entry = entryIt->second;
            _callback(nullptr, std::move(entry));
            return;
        }
    }

    m_prev->asyncGetRow(_tableInfo, _key, _callback);
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
                (*results)[i] = entryIt->second;
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

    if (existsCount < _keys.size())
    {
        m_prev->asyncGetRows(_tableInfo, std::get<0>(*missings),
            [_callback, missings, results](
                Error::Ptr&& error, std::vector<storage::Entry::Ptr>&& entries) {
                if (error)
                {
                    _callback(BCOS_ERROR_WITH_PREV(-1, "async get perv rows failed!", error),
                        std::vector<bcos::storage::Entry::Ptr>());
                    return;
                }

                for (size_t i = 0; i < entries.size(); ++i)
                {
                    (*results)[std::get<1>(*missings)[i]] = entries[i];
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
    auto tableIt = m_data.find(tableInfo->name);
    if (tableIt != m_data.end())
    {
        auto entryIt = tableIt->second.entries.find(key);
        if (entryIt != tableIt->second.entries.end())
        {
            entryIt->second = entry;
        }
        else
        {
            tableIt->second.entries.insert({key, entry});
        }
    }
    else
    {
        asyncOpenTable(tableInfo->name, [this, tableInfo, key, entry, callback](
                                            Error::Ptr&& error, storage::Table::Ptr&& table) {
            if (error)
            {
                callback(
                    BCOS_ERROR_WITH_PREV(-1, "Open table: " + tableInfo->name + " failed", error),
                    false);
                return;
            }

            if (table)
            {
                auto [tableIt, inserted] = m_data.insert({tableInfo->name,
                    {tbb::concurrent_unordered_map<std::string, storage::Entry::Ptr>(), false}});

                if (!inserted)
                {
                    callback(BCOS_ERROR(-1,
                                 "Insert table: " + tableInfo->name + " into tableFactory failed!"),
                        false);
                    return;
                }

                auto entryIt = tableIt->second.entries.find(key);
                if (entryIt != tableIt->second.entries.end())
                {
                    entryIt->second = entry;
                }
                else
                {
                    tableIt->second.entries.insert({key, entry});
                }

                callback(nullptr, true);
            }
            else
            {
                callback(BCOS_ERROR(-1, "Async set row failed, table: " + tableInfo->name +
                                            " does not exists"),
                    false);
            }
        });
    }
}

void TableStorage::asyncPrepare(protocol::BlockNumber,
    const std::shared_ptr<std::vector<storage::TableInfo::Ptr>>&,
    const std::shared_ptr<
        std::vector<std::shared_ptr<std::map<std::string, storage::Entry::Ptr>>>>&,
    std::function<void(Error::Ptr&&)> callback) noexcept
{
    callback(BCOS_ERROR(-1, "Unsupport method"));
}

void TableStorage::aysncCommit(
    protocol::BlockNumber, std::function<void(Error::Ptr&&)> callback) noexcept
{
    callback(BCOS_ERROR(-1, "Unsupport method"));
}

void TableStorage::aysncRollback(
    protocol::BlockNumber, std::function<void(Error::Ptr&&)> callback) noexcept
{
    callback(BCOS_ERROR(-1, "Unsupport method"));
}

void TableStorage::asyncOpenTable(const std::string& tableName,
    std::function<void(Error::Ptr&&, storage::Table::Ptr&&)> callback) noexcept
{
    auto sysTableInfo = getSysTableInfo(tableName);
    if (sysTableInfo)
    {
        auto table = std::make_shared<storage::Table>(
            shared_from_this(), sysTableInfo, m_hashImpl, m_blockNumber);
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
                auto table = std::make_shared<storage::Table>(
                    shared_from_this(), tableInfo, m_hashImpl, m_blockNumber);

                callback(nullptr, std::move(table));
            });
        });
    }
}

void TableStorage::asyncCreateTable(const std::string& _tableName, const std::string& _keyField,
    const std::string& _valueFields, std::function<void(Error::Ptr&&, bool)> callback) noexcept
{
    asyncOpenTable(SYS_TABLES, [this, _tableName, callback, _keyField, _valueFields](
                                   Error::Ptr&& error, storage::Table::Ptr&& sysTable) {
        if (error)
        {
            callback(std::move(error), false);
            return;
        }

        sysTable->asyncGetRow(
            _tableName, [this, _tableName, callback, sysTable, _keyField, _valueFields](
                            Error::Ptr&& error, storage::Entry::Ptr&& entry) {
                if (error)
                {
                    callback(std::move(error), false);
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
                        callback(std::move(error), false);
                        return;
                    }

                    callback(nullptr, true);
                });
            });
    });
}

void TableStorage::rollback(size_t _savepoint)
{
    auto& changeLog = getChangeLog();
    while (_savepoint < changeLog.size())
    {
        auto change = changeLog.back();
        // Public Table API cannot be used here because it will add another change log entry.

        // change->table->rollback(change);
        auto tableIt = m_data.find(change->tableInfo->name);
        if (tableIt != m_data.end())
        {
            auto& tableMap = tableIt->second.entries;
            switch (change->kind)
            {
            case Change::Set:
            {
                if (change->entry)
                {
                    tableMap[change->key] = change->entry;
                }
                else
                {  // nullptr means the key is not exist in m_cache
                    auto oldEntry = std::make_shared<storage::Entry>(nullptr, m_blockNumber);
                    ;
                    oldEntry->setRollbacked(true);
                    tableMap[change->key] = oldEntry;
                }

                m_hash.clear();
                m_dirty = change->tableDirty;
                break;
            }
            case Change::Remove:
            {
                tableMap[change->key]->setStatus(storage::Entry::Status::NORMAL);
                if (change->entry)
                {
                    tableMap[change->key] = change->entry;
                }
                else
                {
                    tableMap[change->key]->setRollbacked(true);
                }
                m_hash.clear();
                m_dirty = change->tableDirty;
                break;
            }
            default:
                break;
            }
        }

        changeLog.pop_back();
    }
}

/*
    crypto::HashType hash()
    {
        std::vector<std::pair<std::string, Table::Ptr>> tables;
        for (auto& it : m_name2Table)
        {
            if (it.second->tableInfo()->enableConsensus && it.second->dirty())
            {
                tables.push_back(std::make_pair(it.first, it.second));
            }
        }
        STORAGE_LOG(DEBUG) << LOG_BADGE("TableFactory hash") << LOG_KV("tables", tables.size());

        tbb::parallel_sort(tables.begin(), tables.end(),
            [](const std::pair<std::string, Table::Ptr>& lhs,
                const std::pair<std::string, Table::Ptr>& rhs) { return lhs.first < rhs.first; });

        bytes data;
        data.resize(tables.size() * crypto::HashType::size);
        tbb::parallel_for(tbb::blocked_range<size_t>(0, tables.size()),
            [&](const tbb::blocked_range<size_t>& range) {
                for (auto it = range.begin(); it != range.end(); ++it)
                {
                    auto table = tables[it];
                    crypto::HashType hash = table.second->hash();
                    if (hash == crypto::HashType())
                    {
                        STORAGE_LOG(DEBUG)
                            << LOG_BADGE("FISCO_DEBUG") << LOG_BADGE("TableFactory hash continued ")
                            << it + 1 << "/" << tables.size() << LOG_KV("tableName", table.first)
                            << LOG_KV("blockNumber", m_blockNumber);
                        continue;
                    }

                    bytes tableHash = hash.asBytes();
                    memcpy(
                        &data[it * crypto::HashType::size], &tableHash[0], crypto::HashType::size);
                    STORAGE_LOG(DEBUG)
                        << LOG_BADGE("FISCO_DEBUG") << LOG_BADGE("TableFactory hash") << it + 1
                        << "/" << tables.size() << LOG_KV("tableName", table.first)
                        << LOG_KV("hash", hash) << LOG_KV("blockNumber", m_blockNumber);
                }
            });

        if (data.empty())
        {
            STORAGE_LOG(DEBUG) << LOG_BADGE("TableFactory empty data");
            return crypto::HashType();
        }
        m_hash = m_hashImpl->hash(&data);
        return m_hash;
    }
    */