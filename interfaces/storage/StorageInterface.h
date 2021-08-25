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
    struct TableData
    {
        TableInfo::Ptr tableInfo;
    };

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

    virtual void asyncRemove(const TableInfo::Ptr& tableInfo, const std::string& key,
        std::function<void(Error::Ptr&&, bool)> callback) noexcept = 0;

    virtual void asyncPrepare(protocol::BlockNumber blockNumber,
        const std::shared_ptr<std::vector<TableInfo::Ptr> >& _infos,
        const std::shared_ptr<std::vector<std::shared_ptr<std::map<std::string, Entry::Ptr> > > >&
            _datas,
        std::function<void(Error::Ptr&&)> callback);

    virtual void aysncCommit(
        protocol::BlockNumber blockNumber, std::function<void(Error::Ptr&&)> callback);

    virtual void aysncRollback(
        protocol::BlockNumber blockNumber, std::function<void(Error::Ptr&&)> callback);
};

}  // namespace storage
}  // namespace bcos
