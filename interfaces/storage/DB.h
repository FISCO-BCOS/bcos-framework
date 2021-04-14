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
 * @brief interface of DB
 * @file DB.h
 * @author: xingqiangbai
 * @date: 2021-04-07
 */
#pragma once
#include "interfaces/storage/Table.h"
#include "libutilities/Error.h"
#include <functional>

namespace bcos
{
namespace storage
{
struct Query : public std::enable_shared_from_this<Query>
{
    using Ptr = std::shared_ptr<Query>;
    std::shared_ptr<TableInfo> tableInfo;
    std::shared_ptr<Condition> condition;
};

class DB : public std::enable_shared_from_this<DB>
{
public:
    using Ptr = std::shared_ptr<DB>;
    DB() = default;
    virtual ~DB() {}
    virtual std::vector<std::string> getPrimaryKeys(std::shared_ptr<Query>& _query) = 0;
    virtual std::shared_ptr<Entry> getRow(
        std::shared_ptr<TableInfo>& _tableInfo, const std::string_view& _key) = 0;
    virtual std::map<std::string, std::shared_ptr<Entry>> getRows(
        std::shared_ptr<TableInfo>& _tableInfo, const std::vector<std::string_view>& _keys) = 0;
    virtual size_t commitTables(
        const std::map<TableInfo, std::map<std::string, std::shared_ptr<Entry>>>& _data) = 0;

    virtual void asyncGetPrimaryKeys(std::shared_ptr<Query>& _query,
        std::function<void(Error, std::vector<std::string>)> _callback) = 0;
    virtual void asyncGetRow(std::shared_ptr<TableInfo>& _tableInfo, const std::string_view& _key,
        std::function<void(Error, std::shared_ptr<Entry>)> _callback) = 0;
    virtual void asyncGetRows(std::shared_ptr<TableInfo>& _tableInfo,
        const std::vector<std::string>& _keys,
        std::function<void(Error, std::map<std::string, std::shared_ptr<Entry>>)> _callback) = 0;
    virtual void asyncCommitTables(
        const std::map<TableInfo, std::map<std::string, std::shared_ptr<Entry>>>& _data,
        std::function<void(Error)> _callback) = 0;
};

}  // namespace storage
}  // namespace bcos
