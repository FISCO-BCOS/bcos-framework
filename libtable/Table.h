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
#include "../interfaces/storage/TableInterface.h"
#include "tbb/concurrent_unordered_map.h"

namespace bcos
{
namespace storage
{
class Table : public TableInterface
{
public:
    using Ptr = std::shared_ptr<Table>;
    Table(std::shared_ptr<StorageInterface> _db, TableInfo::Ptr _tableInfo,
        std::shared_ptr<crypto::Hash> _hashImpl, protocol::BlockNumber _blockNum)
      : m_DB(_db), m_tableInfo(_tableInfo), m_hashImpl(_hashImpl), m_blockNumber(_blockNum)
    {}
    virtual ~Table() {}
    Entry::Ptr getRow(const std::string& _key) override;
    std::map<std::string, Entry::Ptr> getRows(const std::vector<std::string>& _keys) override;
    std::vector<std::string> getPrimaryKeys(const Condition::Ptr& _condition) const override;
    bool setRow(const std::string& _key, const Entry::Ptr& _entry) override;
    bool remove(const std::string& _key) override;

    void asyncGetPrimaryKeys(const Condition::Ptr& _condition,
        std::function<void(const Error::Ptr&, const std::vector<std::string>&)> _callback) override;
    void asyncGetRow(const std::string& _key,
        std::function<void(const Error::Ptr&, const Entry::Ptr&)> _callback) override;
    void asyncGetRows(const std::shared_ptr<std::vector<std::string>>& _keys,
        std::function<void(const Error::Ptr&, const std::map<std::string, Entry::Ptr>&)> _callback)
        override;

    TableInfo::Ptr tableInfo() const override { return m_tableInfo; }
    Entry::Ptr newEntry() override { return std::make_shared<Entry>(m_blockNumber); }
    crypto::HashType hash() override;

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
                    auto entry = std::make_shared<Entry>();
                    entry->copyFrom(it.second);
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
    void rollback(Change::Ptr) override;
    bool dirty() const override { return m_dataDirty; }
    void setRecorder(RecorderType _recorder) override { m_recorder = _recorder; }

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
