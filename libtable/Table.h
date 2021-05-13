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

#include "interfaces/storage/Common.h"
#include "interfaces/storage/StorageInterface.h"
#include "interfaces/storage/TableInterface.h"
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
    std::shared_ptr<Entry> getRow(const std::string& _key) override;
    std::map<std::string, std::shared_ptr<Entry>> getRows(
        const std::vector<std::string>& _keys) override;
    std::vector<std::string> getPrimaryKeys(std::shared_ptr<Condition> _condition) const override;
    bool setRow(const std::string& _key, std::shared_ptr<Entry> _entry) override;
    bool remove(const std::string& _key) override;
    TableInfo::Ptr tableInfo() const override { return m_tableInfo; }
    Entry::Ptr newEntry() override { return std::make_shared<Entry>(m_blockNumber); }
    crypto::HashType hash() override;

    std::shared_ptr<std::map<std::string, std::shared_ptr<Entry>>> dump() override
    {
        auto ret = std::make_shared<std::map<std::string, std::shared_ptr<Entry>>>();

        for (auto& it : m_dirty)
        {
            if (!it.second->rollbacked())
            {
                auto entry = std::make_shared<Entry>();
                entry->copyFrom(it.second);
                (*ret)[it.first] = entry;
            }
        }
        return ret;
    }

    void rollback(Change::Ptr) override;
    bool dirty() const override { return m_dataDirty; }
    void setRecorder(RecorderType _recorder) override { m_recorder = _recorder; }

protected:
    RecorderType m_recorder;
    std::shared_ptr<StorageInterface> m_DB;
    TableInfo::Ptr m_tableInfo;
    tbb::concurrent_unordered_map<std::string, Entry::Ptr> m_dirty;
    std::shared_ptr<crypto::Hash> m_hashImpl;
    protocol::BlockNumber m_blockNumber = 0;
    crypto::HashType m_hash;
    bool m_hashDirty = true;   // mark if m_hash need to re-calculate
    bool m_dataDirty = false;  // mark if table has data to commit
};

}  // namespace storage
}  // namespace bcos
