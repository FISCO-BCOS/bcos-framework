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

    void asyncGetPrimaryKeys(const std::string_view& table,
        const std::optional<storage::Condition const>& _condition,
        std::function<void(std::optional<Error>&&, std::vector<std::string>&&)> _callback) noexcept override;

    void asyncGetRow(const std::string_view& table, const std::string_view& _key,
        std::function<void(std::optional<Error>&&, std::optional<Entry>&&)> _callback) noexcept override;

    void asyncGetRows(const std::string_view& table,
        const std::variant<gsl::span<std::string_view const>, gsl::span<std::string const>>& _keys,
        std::function<void(std::optional<Error>&&, std::vector<std::optional<Entry>>&&)> _callback) noexcept
        override;

    void asyncSetRow(const std::string_view& table, const std::string_view& key, Entry entry,
        std::function<void(std::optional<Error>&&, bool)> callback) noexcept override;

    void parallelTraverse(bool onlyDirty, std::function<bool(const std::string_view& table,
                                              const std::string_view& key, const Entry& entry)>
                                              callback) const override;

    std::optional<Table> openTable(const std::string& table);

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
        std::string table;
        Kind kind;  ///< The kind of the change.
        std::string key;
        std::optional<Entry> entry;
        bool tableDirty;
        Change(std::string _table, Kind _kind, std::string _key, std::optional<Entry> _entry,
            bool _tableDirty)
          : table(std::move(_table)),
            kind(_kind),
            key(std::move(_key)),
            entry(std::move(_entry)),
            tableDirty(_tableDirty)
        {}
    };

    std::vector<Change>& getChangeLog() { return s_changeLog.local(); }

    template <class T>
    void multiAsyncGetRows(const std::string_view& table, const gsl::span<T const>& _keys,
        std::function<void(std::optional<Error>&&, std::vector<std::optional<Entry>>&&)> _callback)
    {
        auto results = std::make_shared<std::vector<std::optional<Entry>>>(_keys.size());
        auto missings = std::make_shared<std::tuple<std::vector<std::string_view>,
            std::vector<std::tuple<std::string, size_t>>>>();

        long existsCount = 0;

        auto tableIt = m_data.find(table);
        if (tableIt != m_data.end())
        {
            size_t i = 0;
            for (auto& key : _keys)
            {
                auto entryIt = std::get<TableData>(tableIt->second).entries.find(key);
                if (entryIt != std::get<TableData>(tableIt->second).entries.end())
                {
                    (*results)[i] = Entry(std::get<Entry>(entryIt->second));
                    ++existsCount;
                }
                else
                {
                    std::get<1>(*missings).emplace_back(std::string(key), i);
                    std::get<0>(*missings).emplace_back(key);
                }

                ++i;
            }
        }
        else
        {
            for (long i = 0; i < _keys.size(); ++i)
            {
                std::get<1>(*missings).emplace_back(std::string(_keys[i]), i);
                std::get<0>(*missings).emplace_back(_keys[i]);
            }
        }

        if (existsCount < _keys.size() && m_prev)
        {
            m_prev->asyncGetRows(table, std::get<0>(*missings),
                [this, _callback, missings, results](auto&& error, auto&& entries) {
                    if (error)
                    {
                        _callback(
                            BCOS_ERROR_WITH_PREV(-1, "async get perv rows failed!", *error),
                            std::vector<std::optional<Entry>>());
                        return;
                    }

                    for (size_t i = 0; i < entries.size(); ++i)
                    {
                        auto& entry = entries[i];

                        if (entry)
                        {
                            (*results)[std::get<1>(std::get<1>(*missings)[i])] =
                                Entry(importExistingEntry(
                                    std::move(std::get<0>(std::get<1>(*missings)[i])),
                                    std::move(*entry)));
                        }
                    }

                    _callback({}, std::move(*results));
                });
        }
        else
        {
            _callback({}, std::move(*results));
        }
    }


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
