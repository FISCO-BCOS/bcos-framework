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
 * @brief interface of TableInterface
 * @file TableInterface.h
 * @author: xingqiangbai
 * @date: 2021-04-07
 */
#pragma once
#include "../../interfaces/crypto/Hash.h"
#include "../../interfaces/storage/Common.h"
#include "../../libutilities/Error.h"
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace bcos
{
namespace storage
{
const char* const SYS_TABLE = "s_tables";
const char* const SYS_TABLE_KEY = "table_name";
const char* const SYS_TABLE_VALUE_FIELDS = "value_fields";
const char* const SYS_TABLE_KEY_FIELDS = "key_field";

inline TableInfo::Ptr getSysTableInfo(const std::string& tableName)
{
    if (tableName == SYS_TABLE)
    {
        return std::make_shared<TableInfo>(tableName, SYS_TABLE_KEY,
            std::string(SYS_TABLE_KEY_FIELDS) + "," + SYS_TABLE_VALUE_FIELDS);
    }
    return nullptr;
}

class TableInterface : public std::enable_shared_from_this<TableInterface>
{
public:
    struct Change
    {
        using Ptr = std::shared_ptr<Change>;
        enum Kind : int
        {
            Set,
            Remove,
        };
        std::shared_ptr<TableInterface> table;
        Kind kind;  ///< The kind of the change.
        std::string key;
        Entry::Ptr entry;
        bool tableDirty;
        Change(std::shared_ptr<TableInterface> _table, Kind _kind, std::string const& _key,
            Entry::Ptr _entry, bool _tableDirty)
          : table(_table), kind(_kind), key(_key), entry(_entry), tableDirty(_tableDirty)
        {}
    };
    using Ptr = std::shared_ptr<TableInterface>;
    using RecorderType = std::function<void(Change::Ptr)>;
    TableInterface() = default;
    virtual ~TableInterface() {}
    virtual std::shared_ptr<Entry> getRow(const std::string& _key) = 0;
    virtual std::map<std::string, std::shared_ptr<Entry>> getRows(
        const std::vector<std::string>& _keys) = 0;
    virtual std::vector<std::string> getPrimaryKeys(
        std::shared_ptr<Condition> _condition) const = 0;
    virtual bool setRow(const std::string& _key, std::shared_ptr<Entry> _entry) = 0;

    virtual void asyncGetPrimaryKeys(std::shared_ptr<Condition> _condition,
        std::function<void(Error::Ptr, std::vector<std::string>)> _callback) = 0;
    virtual void asyncGetRow(std::shared_ptr<std::string> _key,
        std::function<void(Error::Ptr, std::shared_ptr<Entry>)> _callback) = 0;
    virtual void asyncGetRows(std::shared_ptr<std::vector<std::string>> _keys,
        std::function<void(Error::Ptr, std::map<std::string, std::shared_ptr<Entry>>)>
            _callback) = 0;

    virtual bool remove(const std::string& _key) = 0;
    virtual TableInfo::Ptr tableInfo() const = 0;
    virtual Entry::Ptr newEntry() = 0;

    virtual crypto::HashType hash() = 0;

    virtual std::shared_ptr<std::map<std::string, std::shared_ptr<Entry>>> dump() = 0;

    virtual void rollback(Change::Ptr _change) = 0;
    virtual bool dirty() const = 0;
    virtual void setRecorder(RecorderType _recorder) = 0;
};

class TableFactoryInterface : public std::enable_shared_from_this<TableFactoryInterface>
{
public:
    using Ptr = std::shared_ptr<TableFactoryInterface>;
    TableFactoryInterface() = default;
    virtual ~TableFactoryInterface() {}

    virtual std::shared_ptr<TableInterface> openTable(const std::string& _tableName) = 0;
    virtual bool createTable(const std::string& _tableName, const std::string& _keyField,
        const std::string& _valueFields) = 0;

    virtual crypto::HashType hash() = 0;
    virtual size_t savepoint() = 0;
    virtual void rollback(size_t _savepoint) = 0;
    virtual std::pair<size_t, Error::Ptr> commit() = 0;
    virtual void asyncCommit(std::function<void(Error::Ptr, size_t)> _callback) = 0;
    virtual std::pair<std::vector<TableInfo::Ptr>,
        std::vector<std::shared_ptr<std::map<std::string, Entry::Ptr>>>>
    exportData() = 0;
    virtual void importData(std::vector<TableInfo::Ptr>,
        std::vector<std::shared_ptr<std::map<std::string, Entry::Ptr>>>) = 0;
};
}  // namespace storage
}  // namespace bcos
