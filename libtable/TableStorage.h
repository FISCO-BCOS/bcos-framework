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
#include "../libutilities/Error.h"
#include "Table.h"
#include "tbb/concurrent_unordered_map.h"
#include "tbb/enumerable_thread_specific.h"
#include "tbb/parallel_for.h"
#include "tbb/parallel_sort.h"
#include <boost/throw_exception.hpp>
#include <future>
#include <memory>

namespace bcos::storage
{
class TableStorage : public storage::TraverseStorageInterface,
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

    static constexpr const char* SYS_TABLES = "s_tables";
    static constexpr const char* const SYS_TABLE_KEY = "table_name";
    static constexpr const char* const SYS_TABLE_VALUE_FIELDS = "value_fields";
    static constexpr const char* const SYS_TABLE_KEY_FIELDS = "key_field";

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

    // TODO: check if needed
    TableStorage(std::shared_ptr<TraverseStorageInterface> prev,
        std::shared_ptr<StorageInterface> backend, std::shared_ptr<crypto::Hash> _hashImpl,
        protocol::BlockNumber blockNumber, protocol::BlockNumber commitedBlockNumber)
      : m_blockNumber(blockNumber), m_prev(backend), m_hashImpl(_hashImpl)
    {
        if (!prev)
        {
            BOOST_THROW_EXCEPTION(BCOS_ERROR(-1, "Null prev storage"));
        }

        prev->parallelTraverse(false, [&](const TableInfo::Ptr& tableInfo, const std::string& key,
                                          const Entry::ConstPtr& entry) {
            if (entry->num() > commitedBlockNumber)
            {
                importExistingEntry(tableInfo, key, std::make_shared<Entry>(*entry));
            }
            return true;
        });
    }

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
        const storage::Entry::ConstPtr& entry,
        std::function<void(Error::Ptr&&, bool)> callback) noexcept override;

    void parallelTraverse(bool onlyDirty, std::function<bool(const TableInfo::Ptr& tableInfo,
                                              const std::string& key, const Entry::ConstPtr& entry)>
                                              callback) const override;

    void asyncOpenTable(const std::string& tableName,
        std::function<void(Error::Ptr&&, Table::Ptr&&)> callback) noexcept;

    void asyncCreateTable(const std::string& _tableName, const std::string& _keyField,
        const std::string& _valueFields, std::function<void(Error::Ptr&&, bool)> callback) noexcept;

    Table::Ptr openTable(const std::string& tableName);

    bool createTable(const std::string& _tableName, const std::string& _keyField,
        const std::string& _valueFields);

    std::vector<std::tuple<std::string, crypto::HashType>> tablesHash();

    size_t savepoint()
    {
        auto& changeLog = getChangeLog();
        return changeLog.size();
    }
    void rollback(size_t _savepoint);

    protocol::BlockNumber blockNumber() const { return m_blockNumber; }

private:
    std::vector<Change>& getChangeLog() { return s_changeLog.local(); }
    Entry::Ptr importExistingEntry(
        const TableInfo::Ptr& tableInfo, const std::string& key, Entry::Ptr&& entry);

    tbb::enumerable_thread_specific<std::vector<Change>> s_changeLog;

    struct TableData
    {
        TableInfo::Ptr tableInfo;
        tbb::concurrent_unordered_map<std::string, storage::Entry::Ptr> entries;
        bool dirty = false;
    };
    tbb::concurrent_unordered_map<std::string, TableData> m_data;
    protocol::BlockNumber m_blockNumber = 0;

    std::shared_ptr<StorageInterface> m_prev;
    std::shared_ptr<crypto::Hash> m_hashImpl;
};
}  // namespace bcos::storage
