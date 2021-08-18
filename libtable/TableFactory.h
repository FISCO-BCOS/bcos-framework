/*
 *  Copyright (C) 2021 FISCO BCOS.
 *  SPDX-License-Identifier: Apache-2.0
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 * @brief interface of Table
 * @file Table.h
 * @author: xingqiangbai
 * @date: 2021-04-07
 */
#pragma once

#include "Table.h"
#include "tbb/concurrent_unordered_map.h"
#include "tbb/enumerable_thread_specific.h"
#include "tbb/parallel_for.h"
#include "tbb/parallel_sort.h"

namespace bcos
{
namespace storage
{
class TableFactory : public TableFactoryInterface
{
public:
    typedef std::shared_ptr<TableFactory> Ptr;
    TableFactory(std::shared_ptr<StorageInterface> _db, std::shared_ptr<crypto::Hash> _hashImpl,
        protocol::BlockNumber _blockNum)
      : m_DB(_db), m_hashImpl(_hashImpl), m_blockNumber(_blockNum)
    {
        m_sysTables.push_back(SYS_TABLE);
    }
    virtual ~TableFactory()
    {
        m_name2Table.clear();
        getChangeLog().clear();
    }
    // virtual void init();

    std::shared_ptr<TableInterface> openTable(const std::string& _tableName) override
    {
        tbb::spin_mutex::scoped_lock l(x_name2Table);
        return openTableWithoutLock(_tableName);
    }
    bool createTable(const std::string& _tableName, const std::string& _keyField,
        const std::string& _valueFields) override
    {
        // TODO: check tablename and column name characters are permitted
        auto sysTable = openTable(SYS_TABLE);
        // To make sure the table exists
        {
            tbb::spin_mutex::scoped_lock l(x_name2Table);
            auto tableEntry = sysTable->getRow(_tableName);
            if (tableEntry)
            {
                STORAGE_LOG(WARNING) << LOG_BADGE("TableFactory") << LOG_DESC("table already exist")
                                     << LOG_KV("table name", _tableName);
                return false;
            }
            // Write table entry
            tableEntry = sysTable->newEntry();
            tableEntry->setField(SYS_TABLE_KEY, _tableName);
            tableEntry->setField(SYS_TABLE_KEY_FIELDS, _keyField);
            tableEntry->setField(SYS_TABLE_VALUE_FIELDS, _valueFields);
            auto result = sysTable->setRow(_tableName, tableEntry);
            if (!result)
            {
                STORAGE_LOG(WARNING)
                    << LOG_BADGE("TableFactory") << LOG_DESC("create table permission denied")
                    << LOG_KV("table name", _tableName);
                return false;
            }
            // insert new table to m_name2Table
            auto tableInfo =
                std::make_shared<storage::TableInfo>(_tableName, _keyField, _valueFields);
            tableInfo->newTable = true;
            auto table = std::make_shared<Table>(m_DB, tableInfo, m_hashImpl, m_blockNumber);
            table->setRecorder([&](TableInterface::Change::Ptr _change) {
                auto& changeLog = getChangeLog();
                changeLog.emplace_back(_change);
            });

            m_name2Table.insert({_tableName, table});
            // FIXME: write permission info of table
            STORAGE_LOG(DEBUG) << LOG_BADGE("TableFactory") << LOG_DESC("createTable")
                               << LOG_KV("table name", _tableName) << LOG_KV("keyField", _keyField)
                               << LOG_KV("valueFields", _valueFields);
        }
        return true;
    }

    crypto::HashType hash() override
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
    size_t savepoint() override
    {
        auto& changeLog = getChangeLog();
        return changeLog.size();
    }
    void rollback(size_t _savepoint) override
    {
        auto& changeLog = getChangeLog();
        while (_savepoint < changeLog.size())
        {
            auto change = changeLog.back();
            // Public Table API cannot be used here because it will add another change log entry.
            change->table->rollback(change);
            changeLog.pop_back();
            if (change->table->tableInfo()->name == SYS_TABLE)
            {  // if rollback s_tables then should delete the table from m_name2Table
                // the createTable is first option
                tbb::spin_mutex::scoped_lock l(x_name2Table);
                m_name2Table.unsafe_erase(change->key);
            }
        }
    }
    std::pair<size_t, Error::Ptr> commit() override
    {
        auto start_time = utcTime();
        auto record_time = utcTime();
        auto data = exportData(m_blockNumber);
        auto getData_time_cost = utcTime() - record_time;
        record_time = utcTime();
        std::pair<size_t, Error::Ptr> ret{0, nullptr};
        if (!data.second.empty())
        {
            ret = m_DB->commitBlock(m_blockNumber, data.first, data.second);
        }
        auto commit_time_cost = utcTime() - record_time;
        record_time = utcTime();
        m_name2Table.clear();
        auto clear_time_cost = utcTime() - record_time;
        getChangeLog().clear();
        if (m_blockNumber >= 0)
        {
            STORAGE_LOG(DEBUG) << LOG_BADGE("Commit") << LOG_DESC("Commit db time record")
                               << LOG_KV("getDataTimeCost", getData_time_cost)
                               << LOG_KV("commitTimeCost", commit_time_cost)
                               << LOG_KV("clearTimeCost", clear_time_cost)
                               << LOG_KV("totalTimeCost", utcTime() - start_time);
        }
        return ret;
    }

    void asyncCommit(std::function<void(const Error::Ptr&, size_t)> _callback) override
    {
        auto start_time = utcTime();
        auto record_time = utcTime();
        auto data = exportData(m_blockNumber);
        auto getData_time_cost = utcTime() - record_time;
        record_time = utcTime();
        if (!data.second.empty())
        {
            auto dataPtr =
                std::make_shared<std::vector<std::shared_ptr<std::map<std::string, Entry::Ptr>>>>();
            dataPtr->swap(data.second);
            auto infoPtr = std::make_shared<std::vector<TableInfo::Ptr>>();
            infoPtr->swap(data.first);
            m_DB->asyncCommitBlock(m_blockNumber, infoPtr, dataPtr, _callback);
        }
        auto commit_time_cost = utcTime() - record_time;
        if (m_blockNumber >= 0)
        {
            STORAGE_LOG(DEBUG) << LOG_BADGE("Commit") << LOG_DESC("Commit db time record")
                               << LOG_KV("getDataTimeCost", getData_time_cost)
                               << LOG_KV("commitTimeCost", commit_time_cost)
                               << LOG_KV("totalTimeCost", utcTime() - start_time);
        }
    }

    std::pair<std::vector<TableInfo::Ptr>,
        std::vector<std::shared_ptr<std::map<std::string, Entry::Ptr>>>>
    exportData(protocol::BlockNumber blockNumber) override
    {
        std::pair<std::vector<TableInfo::Ptr>,
            std::vector<std::shared_ptr<std::map<std::string, Entry::Ptr>>>>
            ret;
        auto& infos = ret.first;
        auto& datas = ret.second;
        infos.reserve(m_name2Table.size());
        for (auto& dbIt : m_name2Table)
        {
            auto table = dbIt.second;
            if (blockNumber < m_blockNumber || table->dirty())
            {
                infos.push_back(table->tableInfo());
            }
        }
        datas.resize(infos.size());
        tbb::parallel_for(tbb::blocked_range<size_t>(0, infos.size()),
            [&](const tbb::blocked_range<size_t>& range) {
                for (auto it = range.begin(); it != range.end(); ++it)
                {
                    auto table = m_name2Table[infos[it]->name];
                    auto tableData = table->dump(blockNumber);
                    datas[it] = (tableData);
                }
            });
        return ret;
    }

    void importData(std::vector<TableInfo::Ptr>& _tableInfos,
        std::vector<std::shared_ptr<std::map<std::string, Entry::Ptr>>>& _tableDatas,
        bool _dirty = true) override
    {
        tbb::parallel_for(tbb::blocked_range<size_t>(0, _tableDatas.size()),
            [&](const tbb::blocked_range<size_t>& range) {
                for (auto i = range.begin(); i != range.end(); ++i)
                {
                    auto table =
                        std::make_shared<Table>(m_DB, _tableInfos[i], m_hashImpl, m_blockNumber);
                    table->setRecorder([&](TableInterface::Change::Ptr _change) {
                        auto& changeLog = getChangeLog();
                        changeLog.emplace_back(_change);
                    });
                    if (_dirty)
                    {
                        for (auto& item : *(_tableDatas[i]))
                        {
                            table->setRow(item.first, item.second);
                        }
                    }
                    else
                    {
                        _tableInfos[i]->newTable = false;
                        table->importCache(_tableDatas[i]);
                    }
                    m_name2Table.insert({_tableInfos[i]->name, table});
                }
            });
    }

    protocol::BlockNumber blockNumber() const override { return m_blockNumber; }

    bool checkAuthority(const std::string& _tableName, const std::string& _user) const override
    {
        // FIXME: implement when we get permission control
        (void)_tableName;
        (void)_user;
        return true;
    }
    virtual bool grantAccess(const std::string& _tableName, const std::string& _user)
    {
        // FIXME: implement when we get permission control
        (void)_tableName;
        (void)_user;
        return true;
    }

private:
    virtual Table::Ptr openTableWithoutLock(const std::string& tableName)
    {
        auto it = m_name2Table.find(tableName);
        if (it != m_name2Table.end())
        {
            return it->second;
        }

        storage::TableInfo::Ptr tableInfo = nullptr;
        if (m_sysTables.end() != find(m_sysTables.begin(), m_sysTables.end(), tableName))
        {
            tableInfo = getSysTableInfo(tableName);
        }
        else
        {
            auto tempSysTable = openTableWithoutLock(SYS_TABLE);
            auto entry = tempSysTable->getRow(tableName);
            if (!entry)
            {
                return nullptr;
            }
            tableInfo = std::make_shared<storage::TableInfo>(tableName,
                entry->getField(SYS_TABLE_KEY_FIELDS), entry->getField(SYS_TABLE_VALUE_FIELDS));
        }

        auto table = std::make_shared<Table>(m_DB, tableInfo, m_hashImpl, m_blockNumber);
        table->setRecorder([&](Table::Change::Ptr _change) {
            auto& changeLog = getChangeLog();
            changeLog.push_back(_change);
        });

        m_name2Table.insert({tableName, table});
        return table;
    }

    std::vector<Table::Change::Ptr>& getChangeLog() { return s_changeLog.local(); }
    // this map can't be changed, hash() need ordered data
    tbb::concurrent_unordered_map<std::string, Table::Ptr> m_name2Table;
    tbb::enumerable_thread_specific<std::vector<Table::Change::Ptr>> s_changeLog;
    crypto::HashType m_hash;
    std::vector<std::string> m_sysTables;
    std::shared_ptr<StorageInterface> m_DB;
    std::shared_ptr<crypto::Hash> m_hashImpl;
    protocol::BlockNumber m_blockNumber = 0;
    // mutex
    mutable tbb::spin_mutex x_name2Table;
};
}  // namespace storage
}  // namespace bcos
