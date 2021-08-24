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
class TableFactoryInterface;
class StorageInterface : public std::enable_shared_from_this<StorageInterface>
{
public:
    using Ptr = std::shared_ptr<StorageInterface>;
    StorageInterface() = default;
    virtual ~StorageInterface() = default;
    virtual std::vector<std::string> getPrimaryKeys(
        const TableInfo::Ptr& _tableInfo, const Condition::Ptr& _condition) const = 0;
    virtual Entry::Ptr getRow(const TableInfo::Ptr& _tableInfo, const std::string_view& _key) = 0;
    virtual std::map<std::string, Entry::Ptr> getRows(
        const TableInfo::Ptr& _tableInfo, const std::vector<std::string>& _keys) = 0;
    virtual std::pair<size_t, Error::Ptr> commitBlock(protocol::BlockNumber _blockNumber,
        const std::vector<TableInfo::Ptr>& _tableInfos,
        const std::vector<std::shared_ptr<std::map<std::string, Entry::Ptr> > >& _tableDatas) = 0;

    virtual void asyncGetPrimaryKeys(const TableInfo::Ptr& _tableInfo,
        const Condition::Ptr& _condition,
        std::function<void(Error::Ptr&&, std::vector<std::string>&&)> _callback) = 0;

    virtual void asyncGetRow(const TableInfo::Ptr& _tableInfo, const std::string_view& _key,
        std::function<void(Error::Ptr&&, Entry::Ptr&&)> _callback) = 0;

    virtual void asyncGetRows(const TableInfo::Ptr& _tableInfo,
        const std::shared_ptr<std::vector<std::string> >& _keys,
        std::function<void(Error::Ptr&&, std::map<std::string, Entry::Ptr>&&)> _callback) = 0;

    virtual void asyncCommitBlock(protocol::BlockNumber _blockNumber,
        const std::shared_ptr<std::vector<TableInfo::Ptr> >& _infos,
        const std::shared_ptr<std::vector<std::shared_ptr<std::map<std::string, Entry::Ptr> > > >&
            _datas,
        std::function<void(const Error::Ptr&, size_t)> _callback) = 0;

    // cache TableFactoryInterface
    virtual void asyncAddStateCache(protocol::BlockNumber _blockNumber,
        const std::shared_ptr<TableFactoryInterface>& _tablefactory,
        std::function<void(const Error::Ptr&)> _callback) = 0;
    virtual void asyncDropStateCache(
        protocol::BlockNumber _blockNumber, std::function<void(const Error::Ptr&)> _callback) = 0;
    virtual void asyncGetStateCache(protocol::BlockNumber _blockNumber,
        std::function<void(const Error::Ptr&, const std::shared_ptr<TableFactoryInterface>&)>
            _callback) = 0;
    virtual std::shared_ptr<TableFactoryInterface> getStateCache(
        protocol::BlockNumber _blockNumber) = 0;
    virtual void dropStateCache(protocol::BlockNumber _blockNumber) = 0;
    virtual void addStateCache(protocol::BlockNumber _blockNumber,
        const std::shared_ptr<TableFactoryInterface>& _tablefactory) = 0;

    // KV store in split database, used to store data off-chain
    virtual Error::Ptr put(const std::string_view& _columnFamily, const std::string_view& _key,
        const std::string_view& _value) = 0;
    virtual std::pair<std::string, Error::Ptr> get(
        const std::string_view& _columnFamily, const std::string_view& _key) = 0;
    virtual Error::Ptr remove(
        const std::string_view& _columnFamily, const std::string_view& _key) = 0;

    virtual void asyncPut(const std::string_view& _columnFamily, const std::string_view& _key,
        const std::string_view& _value, std::function<void(const Error::Ptr&)> _callback) = 0;
    virtual void asyncRemove(const std::string_view& _columnFamily, const std::string_view& _key,
        std::function<void(const Error::Ptr&)> _callback) = 0;
    virtual void asyncGet(const std::string_view& _columnFamily, const std::string_view& _key,
        std::function<void(const Error::Ptr&, const std::string& value)> _callback) = 0;

    virtual void asyncGetBatch(const std::string_view& _columnFamily,
        const std::shared_ptr<std::vector<std::string> >& _keys,
        std::function<void(const Error::Ptr&, const std::shared_ptr<std::vector<std::string> >&)>
            callback) = 0;
    virtual void stop(){};
    virtual void start(){};
};

}  // namespace storage
}  // namespace bcos
