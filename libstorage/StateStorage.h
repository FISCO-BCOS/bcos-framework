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
#include "../interfaces/storage/Table.h"
#include "../libutilities/Error.h"
#include "tbb/concurrent_unordered_map.h"
#include "tbb/enumerable_thread_specific.h"
#include "tbb/parallel_for.h"
#include "tbb/parallel_sort.h"
#include <boost/throw_exception.hpp>
#include <future>
#include <memory>

namespace bcos::storage
{
class StateStorage : public storage::TraverseStorageInterface,
                     public std::enable_shared_from_this<StateStorage>
{
public:
    typedef std::shared_ptr<StateStorage> Ptr;
    StateStorage(std::shared_ptr<StorageInterface> prev, std::shared_ptr<crypto::Hash> _hashImpl,
        protocol::BlockNumber _blockNum)
      : m_blockNumber(_blockNum), m_prev(std::move(prev)), m_hashImpl(std::move(_hashImpl))
    {}

    // TODO: check if needed
    StateStorage(std::shared_ptr<TraverseStorageInterface> prev,
        std::shared_ptr<StorageInterface> backend, std::shared_ptr<crypto::Hash> _hashImpl,
        protocol::BlockNumber blockNumber, protocol::BlockNumber commitedBlockNumber)
      : m_blockNumber(blockNumber), m_prev(std::move(backend)), m_hashImpl(std::move(_hashImpl))
    {
        if (!prev)
        {
            BOOST_THROW_EXCEPTION(BCOS_ERROR(-1, "Null prev storage"));
        }

        prev->parallelTraverse(false, [&](auto&&, auto&& key, auto&& entry) {
            if (entry.num() > commitedBlockNumber)
            {
                importExistingEntry(std::string(key), std::move(entry));
            }
            return true;
        });
    }

    virtual ~StateStorage() { getChangeLog().clear(); }

    void asyncGetPrimaryKeys(const TableInfo& _tableInfo,
        const std::optional<storage::Condition const>& _condition,
        std::function<void(Error::Ptr&&, std::vector<std::string>&&)> _callback) noexcept override;

    void asyncGetRow(const TableInfo& _tableInfo, const std::string_view& _key,
        std::function<void(Error::Ptr&&, std::optional<Entry>&&)> _callback) noexcept override;

    void asyncGetRows(const TableInfo& _tableInfo, const gsl::span<std::string_view const>& _keys,
        std::function<void(Error::Ptr&&, std::vector<std::optional<Entry>>&&)> _callback) noexcept
        override;

    void asyncSetRow(const storage::TableInfo& tableInfo, const std::string_view& key, Entry entry,
        std::function<void(Error::Ptr&&, bool)> callback) noexcept override;

    void parallelTraverse(bool onlyDirty, std::function<bool(const TableInfo& tableInfo,
                                              const std::string_view& key, const Entry& entry)>
                                              callback) const override;

    std::optional<Table> openTable(const std::string& tableName);

    bool createTable(const std::string& _tableName, const std::string& _keyField,
        const std::string& _valueFields);

    std::vector<std::tuple<std::string, crypto::HashType>> tableHashes();

    size_t savepoint()
    {
        auto& changeLog = getChangeLog();
        return changeLog.size();
    }
    void rollback(size_t _savepoint);

    protocol::BlockNumber blockNumber() const { return m_blockNumber; }

private:
    struct Change
    {
        enum Kind : int
        {
            Set,
            Remove,
        };
        TableInfo::ConstPtr tableInfo;
        Kind kind;  ///< The kind of the change.
        std::string key;
        std::optional<Entry> entry;
        bool tableDirty;
        Change(storage::TableInfo::ConstPtr _tableInfo, Kind _kind, std::string _key,
            std::optional<Entry> _entry, bool _tableDirty)
          : tableInfo(_tableInfo),
            kind(_kind),
            key(std::move(_key)),
            entry(_entry),
            tableDirty(_tableDirty)
        {}
    };

    std::vector<Change>& getChangeLog() { return s_changeLog.local(); }
    Entry& importExistingEntry(std::string key, Entry entry);

    tbb::enumerable_thread_specific<std::vector<Change>> s_changeLog;

    struct TableData
    {
        TableData(TableInfo::ConstPtr tableInfo) : tableInfo(std::move(tableInfo)) {}

        TableInfo::ConstPtr tableInfo;
        tbb::concurrent_unordered_map<std::string_view,
            std::tuple<std::unique_ptr<std::string>, Entry>, std::hash<std::string_view>>
            entries;
        bool dirty = false;
    };
    tbb::concurrent_unordered_map<std::string_view,
        std::tuple<std::unique_ptr<std::string>, TableData>, std::hash<std::string_view>>
        m_data;
    protocol::BlockNumber m_blockNumber = 0;

    std::shared_ptr<StorageInterface> m_prev;
    std::shared_ptr<crypto::Hash> m_hashImpl;
};
}  // namespace bcos::storage
