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
 * @brief interface of Storage
 * @file Storage.h
 * @author: xingqiangbai
 * @date: 2021-04-07
 */
#pragma once
#pragma once
#include "DB.h"
#include <map>
#include <memory>
#include <string>

namespace bcos
{
namespace storage
{
class Storage : public std::enable_shared_from_this<Storage>
{
public:
    using Ptr = std::shared_ptr<Storage>;
    Storage() = default;
    virtual ~Storage() {}
    virtual std::vector<string> getPrimaryKeys(std::shared_ptr<Query>& _query) = 0;
    virtual std::shared_ptr<Entry> getRow(
        std::shared_ptr<TableInfo>& _tableInfo, const std::string_view& _key) = 0;
    virtual std::map<std::string, std::shared_ptr<Entry>> getRows(
        std::shared_ptr<TableInfo>& _tableInfo, const std::vector<std::string_view>& _keys) = 0;
    virtual size_t commitTables(
        const map<TableInfo, std::map<std::string, std::shared_ptr<Entry>>>& _data) = 0;

    virtual void asyncGetPrimaryKeys(std::shared_ptr<Query>& _query,
        std::function<void(Error, std::vector<std::string> >)> _callback) = 0;
    virtual void asyncGetRow(std::shared_ptr<TableInfo>& _tableInfo, const string_view& _key,
        std::function<void(Error, std::shared_ptr<Entry> >)> _callback) = 0;
    virtual void asyncGetRows(std::shared_ptr<TableInfo>& _tableInfo,
        const std::vector<std::string>& _keys,
        std::function<void(Error, std::map<std::string, std::shared_ptr<Entry>>)> _callback) = 0;
    virtual void asyncCommitTables(
        const std::map<TableInfo, std::map<std::string, std::shared_ptr<Entry>>>& _data,
        std::function<void(Error)> _callback) = 0;

    // cache TableFactory
    virtual void asyncAddStateCache(int64_t _blockNumber, protocol::Block::Ptr _block,
        std::shared_ptr<TableFactory> _tablefactory, std::function<void(Error)> _callback) = 0;
    virtual bool asyncDropStateCache(int64_t _blockNumber, std::function<void(Error)> _callback) = 0;
    virtual void asyncGetBlock(
        int64_t _blockNumber, std::function<void(Error, protocol::Block::Ptr)> _callback) = 0;
    virtual void asyncGetStateCache(int64_t _blockNumber,
        std::function<void(Error, std::shared_ptr<TableFactory>)> _callback) = 0;
    virtual protocol::Block::Ptr getBlock(int64_t _blockNumber) = 0;
    virtual std::shared_ptr<TableFactory> getStateCache(int64_t _blockNumber) = 0;
};

}  // namespace storage
}  // namespace bcos
