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
#include <optional>

namespace bcos::storage
{
class StateStorage : public storage::TraverseStorageInterface,
                     public std::enable_shared_from_this<StateStorage>
{
public:
    typedef std::shared_ptr<StateStorage> Ptr;
    StateStorage(std::shared_ptr<StorageInterface> prev) : m_prev(std::move(prev)) {}

    StateStorage(const StateStorage&) = delete;
    StateStorage& operator=(const StateStorage&) = delete;

    StateStorage(StateStorage&&) = default;
    StateStorage& operator=(StateStorage&&) = default;

    virtual ~StateStorage() { m_recoder.clear(); }

    void asyncGetPrimaryKeys(const std::string_view& table,
        const std::optional<storage::Condition const>& _condition,
        std::function<void(Error::UniquePtr&&, std::vector<std::string>&&)> _callback) noexcept
        override;

    void asyncGetRow(const std::string_view& table, const std::string_view& _key,
        std::function<void(Error::UniquePtr&&, std::optional<Entry>&&)> _callback) noexcept
        override;

    void asyncGetRows(const std::string_view& table,
        const std::variant<const gsl::span<std::string_view const>,
            const gsl::span<std::string const>>& _keys,
        std::function<void(Error::UniquePtr&&, std::vector<std::optional<Entry>>&&)>
            _callback) noexcept override;

    void asyncSetRow(const std::string_view& table, const std::string_view& key, Entry entry,
        std::function<void(Error::UniquePtr&&)> callback) noexcept override;

    void parallelTraverse(bool onlyDirty, std::function<bool(const std::string_view& table,
                                              const std::string_view& key, const Entry& entry)>
                                              callback) const override;

    std::optional<Table> openTable(const std::string_view& table);

    std::optional<Table> createTable(std::string _tableName, std::string _valueFields);

    // TODO: change to XOR
    std::vector<std::tuple<std::string, crypto::HashType>> tableHashes(
        const bcos::crypto::Hash::Ptr& hashImpl);

    class Recoder
    {
    public:
        using Ptr = std::shared_ptr<Recoder>;

        struct Change
        {
            Change(std::string_view _table, std::string_view _key, std::optional<Entry> _entry,
                bool _tableDirty)
              : table(_table), key(_key), entry(std::move(_entry)), tableDirty(_tableDirty)
            {}
            Change(const Change&) = delete;
            Change& operator=(const Change&) = delete;
            Change(Change&&) = default;
            Change& operator=(Change&&) = default;

            std::string_view table;
            std::string_view key;
            std::optional<Entry> entry;
            bool tableDirty;
        };

        void log(Change&& change) { m_changes.emplace_front(std::move(change)); }
        auto begin() { return m_changes.begin(); }
        auto end() { return m_changes.end(); }

    private:
        std::list<Change> m_changes;
    };

    Recoder::Ptr newRecoder() { return std::make_shared<Recoder>(); }
    void setRecoder(Recoder::Ptr recoder) { m_recoder.local().swap(recoder); }
    void rollback(const Recoder::Ptr& recoder);

private:
    template <class T>
    void multiAsyncGetRows(const std::string_view& table, const gsl::span<T const>& _keys,
        std::function<void(Error::UniquePtr&&, std::vector<std::optional<Entry>>&&)> _callback)
    {
        auto results = std::make_shared<std::vector<std::optional<Entry>>>(_keys.size());
        auto missinges = std::make_shared<std::tuple<std::vector<std::string_view>,
            std::vector<std::tuple<std::string, size_t>>>>();

        long existsCount = 0;

        auto tableIt = m_data.find(table);
        if (tableIt != m_data.end())
        {
            size_t i = 0;
            for (auto& key : _keys)
            {
                auto entryIt = tableIt->second.entries.find(key);
                if (entryIt != tableIt->second.entries.end())
                {
                    (*results)[i] = Entry(std::get<Entry>(entryIt->second));
                    ++existsCount;
                }
                else
                {
                    std::get<1>(*missinges).emplace_back(std::string(key), i);
                    std::get<0>(*missinges).emplace_back(key);
                }

                ++i;
            }
        }
        else
        {
            for (long i = 0; i < _keys.size(); ++i)
            {
                std::get<1>(*missinges).emplace_back(std::string(_keys[i]), i);
                std::get<0>(*missinges).emplace_back(_keys[i]);
            }
        }

        if (existsCount < _keys.size() && m_prev)
        {
            m_prev->asyncGetRows(table, std::get<0>(*missinges),
                [this, callback = std::move(_callback), missinges, results = std::move(results)](
                    auto&& error, std::vector<std::optional<Entry>>&& entries) {
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
                            (*results)[std::get<1>(std::get<1>(*missinges)[i])] =
                                std::make_optional(importExistingEntry(
                                    std::move(std::get<0>(std::get<1>(*missinges)[i])),
                                    std::move(*entry)));
                        }
                    }

                    callback(nullptr, std::move(*results));
                });
        }
        else
        {
            _callback(nullptr, std::move(*results));
        }
    }

    Entry& importExistingEntry(const std::string_view& key, Entry entry);

    struct TableData
    {
        TableData(TableInfo::ConstPtr tableInfo) : tableInfo(std::move(tableInfo)) {}

        TableInfo::ConstPtr tableInfo;
        tbb::concurrent_unordered_map<std::string_view, std::tuple<std::vector<char>, Entry>,
            std::hash<std::string_view>>
            entries;
        bool dirty = false;
    };

    tbb::concurrent_unordered_map<std::string_view, TableData, std::hash<std::string_view>> m_data;

    // Since transaction is execute in the coroutine, thread local changeLog will be useless
    // tbb::enumerable_thread_specific<std::vector<Change>> s_changeLog;

    tbb::enumerable_thread_specific<Recoder::Ptr> m_recoder;

    std::shared_ptr<StorageInterface> m_prev;
};
}  // namespace bcos::storage
