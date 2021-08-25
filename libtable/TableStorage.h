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

#include "../interfaces/storage/StorageInterface.h"
#include "Table.h"
#include "tbb/concurrent_unordered_map.h"
#include "tbb/enumerable_thread_specific.h"
#include "tbb/parallel_for.h"
#include "tbb/parallel_sort.h"
#include <memory>

namespace bcos::stroage
{
class TableStorage : public storage::StorageInterface,
                     public std::enable_shared_from_this<TableStorage>
{
public:
    struct Change
    {
        enum Kind : int
        {
            Set,
            Remove,
        };
        storage::TableInfo::Ptr tableInfo;
        Kind kind;  ///< The kind of the change.
        std::string key;
        storage::Entry::Ptr entry;
        bool tableDirty;
        Change(storage::TableInfo::Ptr _tableInfo, Kind _kind, std::string const& _key,
            const storage::Entry::Ptr& _entry, bool _tableDirty)
          : tableInfo(_tableInfo), kind(_kind), key(_key), entry(_entry), tableDirty(_tableDirty)
        {}
    };

    const char* const SYS_TABLES = "s_tables";
    const char* const SYS_TABLE_KEY = "table_name";
    const char* const SYS_TABLE_VALUE_FIELDS = "value_fields";
    const char* const SYS_TABLE_KEY_FIELDS = "key_field";

    inline storage::TableInfo::Ptr getSysTableInfo(const std::string& tableName)
    {
        if (tableName == SYS_TABLES)
        {
            return std::make_shared<storage::TableInfo>(tableName, SYS_TABLE_KEY,
                std::string(SYS_TABLE_KEY_FIELDS) + "," + SYS_TABLE_VALUE_FIELDS);
        }
        return nullptr;
    }

    typedef std::shared_ptr<TableStorage> Ptr;
    TableStorage(std::shared_ptr<StorageInterface> prev, std::shared_ptr<crypto::Hash> _hashImpl,
        protocol::BlockNumber _blockNum)
      : m_blockNumber(_blockNum), m_prev(prev), m_hashImpl(_hashImpl)
    {}

    virtual ~TableStorage() { getChangeLog().clear(); }

    void asyncGetPrimaryKeys(const storage::TableInfo::Ptr& _tableInfo,
        const storage::Condition::Ptr& _condition,
        std::function<void(Error::Ptr&&, std::vector<std::string>&&)> _callback) noexcept override;

    void asyncGetRow(const storage::TableInfo::Ptr& _tableInfo, const std::string& _key,
        std::function<void(Error::Ptr&&, storage::Entry::Ptr&&)> _callback) noexcept override;

    void asyncGetRows(const storage::TableInfo::Ptr& _tableInfo,
        const gsl::span<std::string>& _keys,
        std::function<void(Error::Ptr&&, std::vector<storage::Entry::Ptr>&&)> _callback) noexcept
        override;

    void asyncSetRow(const storage::TableInfo::Ptr& tableInfo, const std::string& key,
        const storage::Entry::Ptr& entry,
        std::function<void(Error::Ptr&&, bool)> callback) noexcept override;

    void asyncRemove(const storage::TableInfo::Ptr& tableInfo, const std::string& key,
        std::function<void(Error::Ptr&&, bool)> callback) noexcept override;

    void asyncPrepare(protocol::BlockNumber blockNumber,
        const std::shared_ptr<std::vector<storage::TableInfo::Ptr>>& _infos,
        const std::shared_ptr<
            std::vector<std::shared_ptr<std::map<std::string, storage::Entry::Ptr>>>>& _datas,
        std::function<void(Error::Ptr&&)> callback) noexcept override;

    void aysncCommit(protocol::BlockNumber blockNumber,
        std::function<void(Error::Ptr&&)> callback) noexcept override;

    virtual void aysncRollback(protocol::BlockNumber blockNumber,
        std::function<void(Error::Ptr&&)> callback) noexcept override;

    void asyncOpenTable(const std::string& tableName,
        std::function<void(Error::Ptr&&, storage::Table::Ptr&&)> callback) noexcept;

    void asyncCreateTable(const std::string& _tableName, const std::string& _keyField,
        const std::string& _valueFields, std::function<void(Error::Ptr&&, bool)> callback) noexcept;

    std::tuple<std::string, crypto::HashType> hash();

    size_t savepoint()
    {
        auto& changeLog = getChangeLog();
        return changeLog.size();
    }
    void rollback(size_t _savepoint);

    protocol::BlockNumber blockNumber() const { return m_blockNumber; }

private:
    std::vector<Change>& getChangeLog() { return s_changeLog.local(); }
    tbb::enumerable_thread_specific<std::vector<Change>> s_changeLog;

    struct TableData
    {
        tbb::concurrent_unordered_map<std::string, storage::Entry::Ptr> entries;
        bool dirty = false;
    };
    tbb::concurrent_unordered_map<std::string, TableData> m_data;
    protocol::BlockNumber m_blockNumber = 0;

    crypto::HashType m_hash;
    std::shared_ptr<StorageInterface> m_prev;
    std::shared_ptr<crypto::Hash> m_hashImpl;
    bool m_dirty;
};
}  // namespace bcos::stroage
