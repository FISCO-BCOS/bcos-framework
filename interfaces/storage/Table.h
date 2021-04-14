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
#include "interfaces/crypto/CommonType.h"
#include "interfaces/protocol/Block.h"
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace bcos
{
namespace storage
{
class Entry
{
public:
    using Ptr = std::shared_ptr<Entry>;
    using ConstPtr = std::shared_ptr<const Entry>;
    Entry() = default;
    virtual ~Entry() {}

    virtual std::string getField(const std::string_view& key) const = 0;
    virtual std::string_view getFieldConst(const std::string_view& key) const = 0;
    virtual void setField(const std::string_view& key, const std::string_view& value) = 0;
    virtual std::map<std::string, std::string>::const_iterator find(
        const std::string& key) const = 0;
    virtual std::map<std::string, std::string>::const_iterator begin() const = 0;
    virtual std::map<std::string, std::string>::const_iterator end() const = 0;
    virtual size_t size() const = 0;
    virtual bool isDeleted() const = 0;
    virtual void setDeleted(bool status) = 0;
    virtual ssize_t capacity() const = 0;

    virtual int64_t num() const = 0;
    virtual void setNum(int64_t num) = 0;

    virtual void copyFrom(Entry::ConstPtr entry);

    // TODO: check if we still need these methods
    virtual int64_t getID() const = 0;
    virtual void setID(int64_t id) = 0;
    virtual void setID(const std::string_view& id) = 0;
    virtual bool dirty() const = 0;
    virtual void setDirty(bool dirty) = 0;
    virtual ssize_t capacityOfHashField() const = 0;
    virtual ssize_t refCount() const = 0;  // for test?
};

class Condition : public std::enable_shared_from_this<Condition>
{
public:
    using Ptr = std::shared_ptr<Condition>;
    virtual void EQ(const std::string& key, const std::string& value) = 0;
    virtual void NE(const std::string& key, const std::string& value) = 0;

    virtual void GT(const std::string& key, const std::string& value) = 0;
    virtual void GE(const std::string& key, const std::string& value) = 0;

    virtual void LT(const std::string& key, const std::string& value) = 0;
    virtual void LE(const std::string& key, const std::string& value) = 0;

    virtual bool isValid(const std::string_view& key) const = 0;
};

class Table;
struct Change
{
    enum Kind : int
    {
        Set,
        Remove,
    };
    std::shared_ptr<Table> table;
    Kind kind;  ///< The kind of the change.
    std::string key;
    struct Record
    {
        size_t index;
        std::string key;
        std::string oldValue;
        size_t id;
        Record(size_t _index, std::string _key = std::string(),
            std::string _oldValue = std::string(), size_t _id = 0)
          : index(_index), key(_key), oldValue(_oldValue), id(_id)
        {}
    };
    std::vector<Record> value;
    Change(std::shared_ptr<Table> _table, Kind _kind, std::string const& _key,
        std::vector<Record>& _value)
      : table(_table), kind(_kind), key(_key), value(std::move(_value))
    {}
};

struct TableInfo : public std::enable_shared_from_this<TableInfo>
{
    using Ptr = std::shared_ptr<TableInfo>;
    std::string name;
    std::string key;
    std::vector<std::string> fields;
    std::vector<Address> authorizedAddress;
    std::vector<std::string> indices;

    bool enableConsensus = true;
    bool enableCache = true;
};

class Storage;
class Table : public std::enable_shared_from_this<Table>
{
public:
    using Ptr = std::shared_ptr<Table>;
    Table() = default;
    virtual ~Table() {}
    virtual std::shared_ptr<Entry> getRow(const std::string_view& _key) = 0;
    virtual std::map<std::string, std::shared_ptr<Entry>> getRows(
        const std::vector<std::string>& _keys) = 0;
    virtual std::vector<std::string> getPrimaryKeys(std::shared_ptr<Condition> _condition) = 0;
    virtual bool setRow(const std::string_view& _key, std::shared_ptr<Entry> _entry) = 0;
    virtual bool remove(const std::string_view& _key) = 0;
    virtual crypto::HashType hash() = 0;
    virtual TableInfo::Ptr tableInfo() = 0;

    virtual bool checkAuthority(Address const& _origin) const = 0;
    virtual std::map<std::string, std::shared_ptr<Entry>> dump() = 0;

    virtual void rollback(const Change& _change) = 0;
    virtual void setStateStorage(std::shared_ptr<Storage> _db) = 0;
};

class TableFactory : public std::enable_shared_from_this<TableFactory>
{
public:
    using Ptr = std::shared_ptr<TableFactory>;
    TableFactory() = default;
    virtual ~TableFactory() {}
    virtual void init() = 0;

    virtual std::shared_ptr<Table> openTable(const std::string& _tableName, bool _authority = true) = 0;
    virtual bool createTable(const std::string& _tableName, const std::string& _keyField,
        const std::string& _valueFields) = 0;

    virtual crypto::HashType hash() = 0;
    virtual size_t savepoint() = 0;
    virtual void rollback(size_t _savepoint) = 0;
    virtual void commit(protocol::Block::Ptr _block) = 0;
};
}  // namespace storage
}  // namespace bcos
