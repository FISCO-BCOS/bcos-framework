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
 * @brief interface of StorageInterface
 * @file StorageInterface.h
 * @author: xingqiangbai
 * @date: 2021-04-07
 */
#pragma once
#include "../../interfaces/protocol/Block.h"
#include "../../interfaces/protocol/ProtocolTypeDef.h"
#include "../../libutilities/Error.h"
#include "Common.h"
#include "Entry.h"
#include <map>
#include <memory>
#include <string>

namespace bcos
{
namespace storage
{
class Table;

class StorageInterface
{
public:
    static constexpr const char* SYS_TABLES = "s_tables";
    static constexpr const char* const SYS_TABLE_KEY = "table_name";
    static constexpr const char* const SYS_TABLE_VALUE_FIELDS = "value_fields";
    static constexpr const char* const SYS_TABLE_KEY_FIELDS = "key_field";

    storage::TableInfo::Ptr getSysTableInfo(const std::string& tableName) const;

    using Ptr = std::shared_ptr<StorageInterface>;
    virtual ~StorageInterface() = default;

    virtual void asyncGetPrimaryKeys(const TableInfo::ConstPtr& _tableInfo,
        const std::optional<Condition const>& _condition,
        std::function<void(Error::Ptr&&, std::vector<std::string>&&)> _callback) noexcept = 0;

    virtual void asyncGetRow(const TableInfo::ConstPtr& _tableInfo, const std::string& _key,
        std::function<void(Error::Ptr&&, std::optional<Entry>&&)> _callback) noexcept = 0;

    virtual void asyncGetRows(const TableInfo::ConstPtr& _tableInfo,
        const gsl::span<std::string const>& _keys,
        std::function<void(Error::Ptr&&, std::vector<std::optional<Entry>>&&)>
            _callback) noexcept = 0;

    virtual void asyncSetRow(const TableInfo::ConstPtr& tableInfo, const std::string& key,
        Entry entry, std::function<void(Error::Ptr&&, bool)> callback) noexcept = 0;

    virtual void asyncCreateTable(const std::string& _tableName, const std::string& _keyField,
        const std::string& _valueFields, std::function<void(Error::Ptr&&, bool)> callback) noexcept;

    virtual void asyncOpenTable(const std::string& tableName,
        std::function<void(Error::Ptr&&, std::optional<Table>&&)> callback) noexcept;
};

class TraverseStorageInterface : public StorageInterface
{
public:
    using Ptr = std::shared_ptr<TraverseStorageInterface>;
    using ConstPtr = std::shared_ptr<TraverseStorageInterface const>;
    ~TraverseStorageInterface() override = default;

    virtual void parallelTraverse(bool onlyDirty,
        std::function<bool(
            const TableInfo::ConstPtr& tableInfo, const std::string& key, Entry const& entry)>
            callback) const = 0;
};

class MergeableStorageInterface : public StorageInterface
{
public:
    virtual void merge(const TraverseStorageInterface::ConstPtr& storage) = 0;
};

class TransactionalStorageInterface : public StorageInterface
{
public:
    struct TwoPCParams
    {
        bcos::protocol::BlockNumber number;
    };

    ~TransactionalStorageInterface() override = default;

    virtual void asyncPrepare(const TwoPCParams& params,
        const TraverseStorageInterface::ConstPtr& storage,
        std::function<void(Error::Ptr&&)> callback) noexcept = 0;

    virtual void asyncCommit(
        const TwoPCParams& params, std::function<void(Error::Ptr&&)> callback) noexcept = 0;

    virtual void asyncRollback(
        const TwoPCParams& params, std::function<void(Error::Ptr&&)> callback) noexcept = 0;
};

}  // namespace storage
}  // namespace bcos
