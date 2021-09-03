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

#include "Common.h"
#include "StorageInterface.h"
#include <future>
#include <gsl/span>

namespace bcos
{
namespace storage
{
class Table
{
public:
    Table(StorageInterface* _db, TableInfo::Ptr _tableInfo, protocol::BlockNumber _blockNum)
      : m_storage(_db), m_tableInfo(std::move(_tableInfo)), m_blockNumber(_blockNum)
    {}

    Table(const Table&) = default;
    Table(Table&&) = default;
    Table& operator=(const Table&) = default;
    Table& operator=(Table&&) = default;
    virtual ~Table() {}

    std::optional<Entry> getRow(const std::string& _key);
    std::vector<std::optional<Entry>> getRows(const gsl::span<std::string>& _keys);
    std::vector<std::string> getPrimaryKeys(const Condition& _condition);

    bool setRow(const std::string& _key, Entry _entry);

    void asyncGetPrimaryKeys(Condition const& _condition,
        std::function<void(Error::Ptr&&, std::vector<std::string>&&)> _callback) noexcept;
    void asyncGetRow(const std::string& _key,
        std::function<void(Error::Ptr&&, std::optional<Entry>&&)> _callback) noexcept;
    void asyncGetRows(const gsl::span<std::string>& _keys,
        std::function<void(Error::Ptr&&, std::vector<std::optional<Entry>>&&)> _callback) noexcept;

    void asyncSetRow(const std::string& key, Entry entry,
        std::function<void(Error::Ptr&&, bool)> callback) noexcept;

    TableInfo::Ptr tableInfo() const { return m_tableInfo; }
    Entry newEntry() { return Entry(m_tableInfo, m_blockNumber); }
    Entry newDeletedEntry()
    {
        auto deletedEntry = newEntry();
        deletedEntry.setStatus(Entry::DELETED);
        return deletedEntry;
    }

protected:
    StorageInterface* m_storage;
    TableInfo::Ptr m_tableInfo;
    bcos::protocol::BlockNumber m_blockNumber;
};

}  // namespace storage
}  // namespace bcos
