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
#include <gsl/span>

namespace bcos
{
namespace storage
{
class Table
{
public:
    using Ptr = std::shared_ptr<Table>;

    Table(std::shared_ptr<StorageInterface> _db, TableInfo::Ptr _tableInfo,
        protocol::BlockNumber _blockNum)
      : m_storage(_db), m_tableInfo(_tableInfo), m_blockNumber(_blockNum)
    {}
    virtual ~Table() {}

    Entry::Ptr getRow(const std::string& _key);
    std::vector<Entry::Ptr> getRows(const gsl::span<std::string>& _keys);
    std::vector<std::string> getPrimaryKeys(const Condition::Ptr& _condition);

    bool setRow(const std::string& _key, const Entry::Ptr& _entry);
    bool remove(const std::string& _key);

    void asyncGetPrimaryKeys(const Condition::Ptr& _condition,
        std::function<void(Error::Ptr&&, std::vector<std::string>&&)> _callback) noexcept;
    void asyncGetRow(const std::string& _key,
        std::function<void(Error::Ptr&&, Entry::Ptr&&)> _callback) noexcept;
    void asyncGetRows(const gsl::span<std::string>& _keys,
        std::function<void(Error::Ptr&&, std::vector<Entry::Ptr>&&)> _callback) noexcept;

    void asyncSetRow(const std::string& key, const Entry::Ptr& entry,
        std::function<void(Error::Ptr&&, bool)> callback) noexcept;

    TableInfo::Ptr tableInfo() const { return m_tableInfo; }
    Entry::Ptr newEntry() { return std::make_shared<Entry>(m_tableInfo, m_blockNumber); }

protected:
    std::shared_ptr<StorageInterface> m_storage;
    TableInfo::Ptr m_tableInfo;
    bcos::protocol::BlockNumber m_blockNumber;
};

}  // namespace storage
}  // namespace bcos
