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
class StorageInterface
{
public:
    using Ptr = std::shared_ptr<StorageInterface>;
    virtual ~StorageInterface() = default;

    virtual void asyncGetPrimaryKeys(const TableInfo::Ptr& _tableInfo,
        const Condition::Ptr& _condition,
        std::function<void(Error::Ptr&&, std::vector<std::string>&&)> _callback) noexcept = 0;

    virtual void asyncGetRow(const TableInfo::Ptr& _tableInfo, const std::string& _key,
        std::function<void(Error::Ptr&&, Entry::Ptr&&)> _callback) noexcept = 0;

    virtual void asyncGetRows(const TableInfo::Ptr& _tableInfo, const gsl::span<std::string>& _keys,
        std::function<void(Error::Ptr&&, std::vector<Entry::Ptr>&&)> _callback) noexcept = 0;

    virtual void asyncSetRow(const TableInfo::Ptr& tableInfo, const std::string& key,
        const Entry::Ptr& entry, std::function<void(Error::Ptr&&, bool)> callback) noexcept = 0;
};

class TraverseStorageInterface : public StorageInterface
{
public:
    using Ptr = std::shared_ptr<TraverseStorageInterface>;
    ~TraverseStorageInterface() override = default;

    virtual void parallelTraverse(bool onlyDirty,
        std::function<bool(
            const TableInfo::Ptr& tableInfo, const std::string& key, const Entry::ConstPtr& entry)>
            callback) const = 0;
};

class MergeableStorageInterface : public StorageInterface
{
public:
    virtual void merge(const TraverseStorageInterface::Ptr& storage) = 0;
};

class TransactionalStorageInterface : public StorageInterface
{
public:
    struct PrepareParams {

    };

    ~TransactionalStorageInterface() override = default;

    virtual void asyncPrepare(const PrepareParams &params, const TraverseStorageInterface::Ptr& storage,
        std::function<void(Error::Ptr&&)> callback) noexcept = 0;

    virtual void aysncCommit(
        protocol::BlockNumber blockNumber, std::function<void(Error::Ptr&&)> callback) noexcept = 0;

    virtual void aysncRollback(
        protocol::BlockNumber blockNumber, std::function<void(Error::Ptr&&)> callback) noexcept = 0;
};

}  // namespace storage
}  // namespace bcos
