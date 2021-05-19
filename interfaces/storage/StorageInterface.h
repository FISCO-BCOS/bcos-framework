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
#include <map>
#include <memory>
#include <string>

namespace bcos
{
namespace storage
{
class TableFactory;
class StorageInterface : public std::enable_shared_from_this<StorageInterface>
{
public:
    enum ErrorCode
    {
        DataBaseUnavailable = -50000,
        NotFound = -50001,
    };
    using Ptr = std::shared_ptr<StorageInterface>;
    StorageInterface() = default;
    virtual ~StorageInterface() = default;
    virtual std::vector<std::string> getPrimaryKeys(
        std::shared_ptr<TableInfo> _tableInfo, std::shared_ptr<Condition> _condition) const = 0;
    virtual std::shared_ptr<Entry> getRow(
        std::shared_ptr<TableInfo> _tableInfo, const std::string_view& _key) = 0;
    virtual std::map<std::string, std::shared_ptr<Entry> > getRows(
        std::shared_ptr<TableInfo> _tableInfo, const std::vector<std::string>& _keys) = 0;
    virtual std::pair<size_t, Error::Ptr> commitBlock(protocol::BlockNumber _blockNumber,
        const std::vector<std::shared_ptr<TableInfo> > _tableInfos,
        std::vector<std::shared_ptr<std::map<std::string, std::shared_ptr<Entry> > > >&
            _tableDatas) = 0;

    virtual void asyncGetPrimaryKeys(std::shared_ptr<TableInfo> _tableInfo,
        std::shared_ptr<Condition> _condition,
        std::function<void(Error::Ptr, std::vector<std::string>)> _callback) = 0;
    virtual void asyncGetRow(std::shared_ptr<TableInfo> _tableInfo,
        std::shared_ptr<std::string> _key,
        std::function<void(Error::Ptr, std::shared_ptr<Entry>)> _callback) = 0;
    virtual void asyncGetRows(std::shared_ptr<TableInfo> _tableInfo,
        std::shared_ptr<std::vector<std::string> > _keys,
        std::function<void(Error::Ptr, std::map<std::string, std::shared_ptr<Entry> >)>
            _callback) = 0;
    virtual void asyncCommitBlock(protocol::BlockNumber _blockNumber,
        std::shared_ptr<std::vector<std::shared_ptr<TableInfo> > > _infos,
        std::shared_ptr<std::vector<std::shared_ptr<std::map<std::string, Entry::Ptr> > > >& _datas,
        std::function<void(Error::Ptr, size_t)> _callback) = 0;

    // cache TableFactory
    virtual void asyncAddStateCache(protocol::BlockNumber _blockNumber,
        std::shared_ptr<TableFactory> _tablefactory, std::function<void(Error::Ptr)> _callback) = 0;
    virtual void asyncDropStateCache(
        protocol::BlockNumber _blockNumber, std::function<void(Error::Ptr)> _callback) = 0;
    virtual void asyncGetStateCache(protocol::BlockNumber _blockNumber,
        std::function<void(Error::Ptr, std::shared_ptr<TableFactory>)> _callback) = 0;
    virtual std::shared_ptr<TableFactory> getStateCache(protocol::BlockNumber _blockNumber) = 0;
    virtual void dropStateCache(protocol::BlockNumber _blockNumber) = 0;
    virtual void addStateCache(
        protocol::BlockNumber _blockNumber, std::shared_ptr<TableFactory> _tablefactory) = 0;
    // KV store in split database, used to store data off-chain
    virtual Error::Ptr put(const std::string_view& _columnFamily, const std::string_view& _key,
        const std::string_view& _value) = 0;
    virtual std::pair<std::string, Error::Ptr> get(
        const std::string_view& _columnFamily, const std::string_view& _key) = 0;
    virtual Error::Ptr remove(
        const std::string_view& _columnFamily, const std::string_view& _key) = 0;

    virtual void asyncPut(std::shared_ptr<std::string> _columnFamily,
        std::shared_ptr<std::string> _key, std::shared_ptr<bytes> _value,
        std::function<void(Error::Ptr)> _callback) = 0;
    virtual void asyncRemove(std::shared_ptr<std::string> _columnFamily,
        std::shared_ptr<std::string> _key, std::function<void(Error::Ptr)> _callback) = 0;
    virtual void asyncGet(std::shared_ptr<std::string> _columnFamily,
        std::shared_ptr<std::string> _key,
        std::function<void(Error::Ptr, const std::string& value)> _callback) = 0;

    virtual void asyncGetBatch(std::shared_ptr<std::string> _columnFamily,
        std::shared_ptr<std::vector<std::string> > _keys,
        std::function<void(Error::Ptr, std::shared_ptr<std::vector<std::string> >)> callback) = 0;
};

}  // namespace storage
}  // namespace bcos
