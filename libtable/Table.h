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

#include "../interfaces/storage/Common.h"
#include "../interfaces/storage/StorageInterface.h"
#include "tbb/concurrent_unordered_map.h"
#include <future>

namespace bcos
{
namespace storage
{
const char* const SYS_TABLE = "s_tables";
const char* const SYS_TABLE_KEY = "table_name";
const char* const SYS_TABLE_VALUE_FIELDS = "value_fields";
const char* const SYS_TABLE_KEY_FIELDS = "key_field";

inline TableInfo::Ptr getSysTableInfo(const std::string& tableName)
{
    if (tableName == SYS_TABLE)
    {
        return std::make_shared<TableInfo>(tableName, SYS_TABLE_KEY,
            std::string(SYS_TABLE_KEY_FIELDS) + "," + SYS_TABLE_VALUE_FIELDS);
    }
    return nullptr;
}

class Table
{
public:
    struct Change
    {
        using Ptr = std::shared_ptr<Change>;
        enum Kind : int
        {
            Set,
            Remove,
        };
        std::shared_ptr<Table> table;
        Kind kind;  ///< The kind of the change.
        std::string key;
        Entry::Ptr entry;
        bool tableDirty;
        Change(std::shared_ptr<Table> _table, Kind _kind, std::string const& _key,
            const Entry::Ptr& _entry, bool _tableDirty)
          : table(_table), kind(_kind), key(_key), entry(_entry), tableDirty(_tableDirty)
        {}
    };
    using RecorderType = std::function<void(Change::Ptr)>;

    using Ptr = std::shared_ptr<Table>;
    Table(std::shared_ptr<StorageInterface> _db, TableInfo::Ptr _tableInfo,
        std::shared_ptr<crypto::Hash> _hashImpl, protocol::BlockNumber _blockNum)
      : m_DB(_db), m_tableInfo(_tableInfo), m_hashImpl(_hashImpl), m_blockNumber(_blockNum)
    {}
    virtual ~Table() {}

    Entry::Ptr getRow(const std::string& _key)
    {
        std::promise<std::tuple<Error::Ptr, Entry::Ptr>> promise;

        asyncGetRow(_key, [&promise](Error::Ptr&& error, Entry::Ptr&& entry) {
            promise.set_value(std::tuple{std::move(error), std::move(entry)});
        });

        auto result = promise.get_future().get();

        if (std::get<0>(result))
        {
            BOOST_THROW_EXCEPTION(*(std::get<0>(result)));
        }

        return std::get<1>(result);
    }

    std::map<std::string, Entry::Ptr> getRows(const std::vector<std::string>& _keys)
    {
        std::promise<std::tuple<Error::Ptr, std::map<std::string, Entry::Ptr>>> promise;
        asyncGetRows(std::make_shared<std::vector<std::string>>(_keys),
            [&promise](Error::Ptr&& error, std::map<std::string, Entry::Ptr>&& entries) {
                promise.set_value(std::tuple{std::move(error), std::move(entries)});
            });

        auto result = promise.get_future().get();

        if (std::get<0>(result))
        {
            BOOST_THROW_EXCEPTION(*(std::get<0>(result)));
        }

        return std::get<1>(result);
    }

    std::vector<std::string> getPrimaryKeys(const Condition::Ptr& _condition)
    {
        std::promise<std::tuple<Error::Ptr, std::vector<std::string>>> promise;
        asyncGetPrimaryKeys(
            _condition, [&promise](Error::Ptr&& error, std::vector<std::string>&& keys) {
                promise.set_value(std::tuple{std::move(error), std::move(keys)});
            });
        auto result = promise.get_future().get();

        if (std::get<0>(result))
        {
            BOOST_THROW_EXCEPTION(*(std::get<0>(result)));
        }

        return std::get<1>(result);
    }

    bool setRow(const std::string& _key, const Entry::Ptr& _entry);
    bool remove(const std::string& _key);

    void asyncGetPrimaryKeys(const Condition::Ptr& _condition,
        std::function<void(Error::Ptr&&, std::vector<std::string>&&)> _callback);
    void asyncGetRow(
        const std::string& _key, std::function<void(Error::Ptr&&, Entry::Ptr&&)> _callback);
    void asyncGetRows(const std::shared_ptr<std::vector<std::string>>& _keys,
        std::function<void(Error::Ptr&&, std::map<std::string, Entry::Ptr>&&)> _callback);

    TableInfo::Ptr tableInfo() const { return m_tableInfo; }
    Entry::Ptr newEntry() { return std::make_shared<Entry>(m_tableInfo, m_blockNumber); }
    crypto::HashType hash();

    /*
    std::shared_ptr<std::map<std::string, Entry::Ptr>> dump(
        protocol::BlockNumber blockNumber) override
    {
        auto ret = std::make_shared<std::map<std::string, Entry::Ptr>>();
        bool onlyDirty = (m_blockNumber == blockNumber);
        for (auto& it : m_cache)
        {
            if (!it.second->rollbacked())
            {
                if ((onlyDirty && it.second->dirty()) ||
                    (!onlyDirty && it.second->num() >= blockNumber))
                {
                    auto entry = std::make_shared<Entry>(*(it.second));
                    (*ret)[it.first] = entry;
                }
            }
        }
        return ret;
    }

    void importCache(const std::shared_ptr<std::map<std::string, Entry::Ptr>>& _tableData) override
    {
        for (auto& item : *_tableData)
        {
            m_cache[item.first] = item.second;
            item.second->setDirty(false);
        }
        m_tableInfo->newTable = false;
    }
    */

    void rollback(Change::Ptr _change);
    bool dirty() const { return m_dataDirty; }
    void setRecorder(RecorderType _recorder) { m_recorder = _recorder; }

protected:
    RecorderType m_recorder;
    std::shared_ptr<StorageInterface> m_DB;
    TableInfo::Ptr m_tableInfo;
    tbb::concurrent_unordered_map<std::string, Entry::Ptr> m_cache;
    std::shared_ptr<crypto::Hash> m_hashImpl;
    protocol::BlockNumber m_blockNumber = 0;
    crypto::HashType m_hash;
    bool m_hashDirty = true;   // mark if m_hash need to re-calculate
    bool m_dataDirty = false;  // mark if table has data to commit
};

}  // namespace storage
}  // namespace bcos
