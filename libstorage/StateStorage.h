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
 * @brief interface of Table
 * @file StateStorage.h
 * @author: ancelmo
 * @date: 2021-09-01
 */
#pragma once

#include "../interfaces/storage/StorageInterface.h"
#include "../interfaces/storage/Table.h"
#include "../libutilities/Error.h"
#include "tbb/concurrent_unordered_map.h"
#include "tbb/enumerable_thread_specific.h"
#include "tbb/parallel_for.h"
#include "tbb/parallel_sort.h"
#include <tbb/concurrent_hash_map.h>
#include <boost/throw_exception.hpp>
#include <future>
#include <memory>
#include <optional>

namespace bcos::storage
{
class StateStorage : public virtual storage::TraverseStorageInterface
{
public:
    using Ptr = std::shared_ptr<StateStorage>;

    explicit StateStorage(std::shared_ptr<StorageInterface> prev) : m_prev(std::move(prev)) {}

    StateStorage(const StateStorage&) = delete;
    StateStorage& operator=(const StateStorage&) = delete;

    StateStorage(StateStorage&&) = default;
    StateStorage& operator=(StateStorage&&) = default;

    virtual ~StateStorage() { m_recoder.clear(); }

    void asyncGetPrimaryKeys(const std::string_view& table,
        const std::optional<storage::Condition const>& _condition,
        std::function<void(Error::UniquePtr, std::vector<std::string>)> _callback) override;

    void asyncGetRow(const std::string_view& table, const std::string_view& _key,
        std::function<void(Error::UniquePtr, std::optional<Entry>)> _callback) override;

    void asyncGetRows(const std::string_view& table,
        const std::variant<const gsl::span<std::string_view const>,
            const gsl::span<std::string const>>& _keys,
        std::function<void(Error::UniquePtr, std::vector<std::optional<Entry>>)> _callback)
        override;

    void asyncSetRow(const std::string_view& table, const std::string_view& key, Entry entry,
        std::function<void(Error::UniquePtr)> callback) override;

    void parallelTraverse(bool onlyDirty, std::function<bool(const std::string_view& table,
                                              const std::string_view& key, const Entry& entry)>
                                              callback) const override;

    std::optional<Table> openTable(const std::string_view& table);

    std::optional<Table> createTable(std::string _tableName, std::string _valueFields);

    crypto::HashType hash(const bcos::crypto::Hash::Ptr& hashImpl);

    size_t capacity() const { return m_capacity; }

    void setPrev(std::shared_ptr<StorageInterface> prev) { m_prev = std::move(prev); }

    class Recoder
    {
    public:
        using Ptr = std::shared_ptr<Recoder>;
        using ConstPtr = std::shared_ptr<Recoder>;

        struct Change
        {
            Change(std::string_view _table, std::string _key, std::optional<Entry> _entry)
              : table(_table), key(std::move(_key)), entry(std::move(_entry))
            {}
            Change(const Change&) = delete;
            Change& operator=(const Change&) = delete;
            Change(Change&&) noexcept = default;
            Change& operator=(Change&&) noexcept = default;

            std::string_view table;
            std::string key;
            std::optional<Entry> entry;
        };

        void log(Change&& change) { m_changes.emplace_front(std::move(change)); }
        auto begin() { return m_changes.begin(); }
        auto end() { return m_changes.end(); }

    private:
        std::list<Change> m_changes;
    };

    Recoder::Ptr newRecoder() { return std::make_shared<Recoder>(); }
    void setRecoder(Recoder::Ptr recoder) { m_recoder.local().swap(recoder); }
    void rollback(const Recoder::ConstPtr& recoder);

    void setEnableTraverse(bool enableTraverse) { m_enableTraverse = enableTraverse; }
    void setCachePrev(bool cachePrev) { m_cachePrev = cachePrev; }

protected:
    class EntryKey
    {
    public:
        EntryKey() {}
        EntryKey(std::string_view table, std::string_view key) : m_table(table), m_key(key) {}
        EntryKey(std::string_view table, std::string key) : m_table(table), m_key(std::move(key)) {}

        EntryKey(const EntryKey&) = default;
        EntryKey& operator=(const EntryKey&) = default;
        EntryKey(EntryKey&&) noexcept = default;
        EntryKey& operator=(EntryKey&&) noexcept = default;

        std::string_view table() const { return m_table; }

        std::string_view key() const
        {
            std::string_view view;
            std::visit([&view](auto&& key) { view = key; }, m_key);

            return view;
        }

        bool operator==(const EntryKey& rhs) const
        {
            return m_table == rhs.m_table && key() == rhs.key();
        }

        bool operator<(const EntryKey& rhs) const
        {
            if (m_table != rhs.m_table)
            {
                return m_table < rhs.m_table;
            }

            return m_key < rhs.m_key;
        }

    private:
        std::string_view m_table;
        std::variant<std::string_view, std::string> m_key;
    };

    struct EntryKeyHasher
    {
        size_t hash(const EntryKey& dataKey) const
        {
            size_t seed = hashString(dataKey.table());
            boost::hash_combine(seed, hashString(dataKey.key()));

            return seed;
        }

        bool equal(const EntryKey& lhs, const EntryKey& rhs) const
        {
            return lhs.table() == rhs.table() && lhs.key() == rhs.key();
        }

        std::hash<std::string_view> hashString;
    };

    using TableKey = std::string_view;

    struct TableKeyHasher
    {
        size_t hash(const TableKey& dataKey) const { return hashString(dataKey); }
        bool equal(const TableKey& lhs, const TableKey& rhs) const { return lhs == rhs; }

        std::hash<std::string_view> hashString;
    };

private:
    Entry importExistingEntry(const std::string_view& key, Entry entry);

    tbb::concurrent_hash_map<EntryKey, Entry, EntryKeyHasher> m_data;
    tbb::concurrent_hash_map<TableKey, TableInfo::ConstPtr, TableKeyHasher> m_tableInfos;

    tbb::enumerable_thread_specific<Recoder::Ptr> m_recoder;
    std::shared_ptr<StorageInterface> m_prev;
    size_t m_capacity = 0;
    bool m_enableTraverse = false;
    bool m_cachePrev = true;
};
}  // namespace bcos::storage
