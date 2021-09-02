/**
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
 * @brief Unit tests for the Table
 * @file Table.cpp
 */

#include "libtable/Table.h"
#include "../../../testutils/TestPromptFixture.h"
#include "Hash.h"
#include "interfaces/crypto/CommonType.h"
#include "libtable/TableStorage.h"
#include "libutilities/ThreadPool.h"
#include <boost/test/tools/old/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <string>

using namespace std;
using namespace bcos;
using namespace bcos::storage;
using namespace bcos::crypto;

namespace std
{
ostream& operator<<(ostream& os, const tuple<string, crypto::HashType>& item)
{
    os << get<0>(item) << " " << get<1>(item);
    return os;
}
}  // namespace std

namespace bcos
{
namespace test
{
struct TableFixture
{
    TableFixture()
    {
        hashImpl = make_shared<Header256Hash>();
        memoryStorage = make_shared<TableStorage>(nullptr, hashImpl, 0);
        tableFactory = make_shared<TableStorage>(memoryStorage, hashImpl, m_blockNumber);
    }

    ~TableFixture() {}
    std::shared_ptr<crypto::Hash> hashImpl = nullptr;
    std::shared_ptr<StorageInterface> memoryStorage = nullptr;
    protocol::BlockNumber m_blockNumber = 0;
    std::shared_ptr<TableStorage> tableFactory = nullptr;
};
BOOST_FIXTURE_TEST_SUITE(TableTest, TableFixture)

BOOST_AUTO_TEST_CASE(constructor)
{
    auto threadPool = ThreadPool("a", 1);
    auto table = std::make_shared<Table>(nullptr, nullptr, 0);
    auto tableFactory = std::make_shared<TableStorage>(memoryStorage, hashImpl, 0);
}

BOOST_AUTO_TEST_CASE(dump_hash)
{
    std::string tableName("t_test");
    std::string keyField("key");
    std::string valueField("value");

    std::promise<bool> createPromise;
    tableFactory->asyncCreateTable(
        tableName, keyField, valueField, [&](Error::Ptr&& error, bool success) {
            BOOST_CHECK_EQUAL(error, nullptr);
            createPromise.set_value(success);
        });

    BOOST_CHECK_EQUAL(createPromise.get_future().get(), true);

    std::promise<Table::Ptr> tablePromise;
    tableFactory->asyncOpenTable("t_test", [&](Error::Ptr&& error, Table::Ptr&& table) {
        BOOST_CHECK_EQUAL(error, nullptr);
        tablePromise.set_value(std::move(table));
    });
    auto table = tablePromise.get_future().get();
    BOOST_CHECK_NE(table, nullptr);

    // BOOST_TEST(table->dirty() == false);
    auto entry = table->newEntry();
    // entry->setField("key", "name");
    entry->setField("value", "Lili");
    table->setRow("name", entry);
    auto tableinfo = table->tableInfo();
    BOOST_CHECK_EQUAL(tableinfo->name, tableName);

    // BOOST_CHECK_EQUAL_COLLECTIONS(
    //     valueField.begin(), valueField.end(), tableinfo->fields.begin(),
    //     tableinfo->fields.end());

    auto hash = tableFactory->tableHashes();
    BOOST_CHECK_EQUAL(hash.size(), 2);  // include s_tables and t_test
    BOOST_CHECK_EQUAL(std::get<1>(hash[0]).size, 32);

    // BOOST_CHECK_EQUAL(tableFactory.ex)

    // auto data = table->dump(m_blockNumber);
    // auto hash = table->hash();
    // BOOST_TEST(data->size() == 1);
    entry = table->newEntry();
    // entry->setField("key", "name2");
    entry->setField("value", "WW");
    BOOST_CHECK_EQUAL(table->setRow("name2", entry), true);

    // data = table->dump(m_blockNumber);
    // BOOST_TEST(data->size() == 2);
    // hash = table->hash();
    // BOOST_TEST(table->dirty() == true);
}

BOOST_AUTO_TEST_CASE(setRow)
{
    std::string tableName("t_test");
    std::string keyField("key");
    std::string valueField("value1,value2");

    std::promise<bool> createPromise;
    tableFactory->asyncCreateTable(
        tableName, keyField, valueField, [&](Error::Ptr&& error, bool success) {
            BOOST_CHECK_EQUAL(error, nullptr);
            createPromise.set_value(success);
        });
    BOOST_CHECK_EQUAL(createPromise.get_future().get(), true);

    std::promise<Table::Ptr> tablePromise;
    tableFactory->asyncOpenTable("t_test", [&](Error::Ptr&& error, Table::Ptr&& table) {
        BOOST_CHECK_EQUAL(error, nullptr);
        tablePromise.set_value(std::move(table));
    });
    auto table = tablePromise.get_future().get();
    BOOST_CHECK_NE(table, nullptr);

    // check fields order of t_test
    BOOST_TEST(table->tableInfo()->fields.size() == 2);
    BOOST_TEST(table->tableInfo()->fields[0] == "value1");
    BOOST_TEST(table->tableInfo()->fields[1] == "value2");
    BOOST_TEST(table->tableInfo()->key == keyField);
    auto entry = table->newEntry();
    // entry->setField("key", "name");
    BOOST_CHECK_THROW(entry->setField("value", "Lili"), bcos::Error);
    BOOST_CHECK_THROW(entry->setField("invalid", "name"), bcos::Error);
    auto ret = table->setRow("name", entry);
    BOOST_CHECK_EQUAL(ret, true);

    // check fields order of SYS_TABLE
    std::promise<Table::Ptr> sysTablePromise;
    tableFactory->asyncOpenTable(
        TableStorage::SYS_TABLES, [&](Error::Ptr&& error, Table::Ptr&& table) {
            BOOST_CHECK_EQUAL(error, nullptr);
            BOOST_CHECK_NE(table, nullptr);
            sysTablePromise.set_value(std::move(table));
        });
    auto sysTable = sysTablePromise.get_future().get();
    BOOST_CHECK_NE(sysTable, nullptr);

    BOOST_TEST(sysTable->tableInfo()->fields.size() == 2);
    BOOST_TEST(sysTable->tableInfo()->fields[0] == TableStorage::SYS_TABLE_KEY_FIELDS);
    BOOST_TEST(sysTable->tableInfo()->fields[1] == TableStorage::SYS_TABLE_VALUE_FIELDS);
    BOOST_TEST(sysTable->tableInfo()->key == TableStorage::SYS_TABLE_KEY);
}

BOOST_AUTO_TEST_CASE(removeFromCache)
{
    std::string tableName("t_test");
    std::string keyField("key");
    std::string valueField("value1,value2");

    auto ret = tableFactory->createTable(tableName, keyField, valueField);
    BOOST_TEST(ret == true);
    auto table = tableFactory->openTable("t_test");
    BOOST_TEST(table != nullptr);
    // check fields order of t_test
    BOOST_TEST(table->tableInfo()->fields.size() == 2);
    BOOST_TEST(table->tableInfo()->fields[0] == "value1");
    BOOST_TEST(table->tableInfo()->fields[1] == "value2");
    BOOST_TEST(table->tableInfo()->key == keyField);
    auto entry = table->newEntry();
    // entry->setField("key", "name");
    entry->setField("value1", "hello world!");
    entry->setField("value2", "hello world2!");
    BOOST_CHECK_THROW(entry->setField("value", "Lili"), bcos::Error);
    BOOST_CHECK_THROW(entry->setField("invalid", "name"), bcos::Error);
    BOOST_CHECK_EQUAL(table->setRow("name", entry), true);

    auto deleteEntry = table->newEntry();
    deleteEntry->setStatus(Entry::DELETED);
    deleteEntry->setVersion(entry->version() + 1);
    BOOST_CHECK_EQUAL(table->setRow("name", deleteEntry), true);

    auto hashs = tableFactory->tableHashes();

    auto tableFactory2 = std::make_shared<TableStorage>(nullptr, hashImpl, 0);
    BOOST_CHECK_EQUAL(tableFactory2->createTable(tableName, keyField, valueField), true);
    auto table2 = tableFactory2->openTable(tableName);
    BOOST_CHECK_NE(table2, nullptr);

    auto deleteEntry2 = table2->newEntry();
    deleteEntry2->setStatus(Entry::DELETED);
    BOOST_CHECK_EQUAL(table2->setRow("name", deleteEntry2), true);
    auto hashs2 = tableFactory2->tableHashes();

    BOOST_CHECK_EQUAL_COLLECTIONS(hashs.begin(), hashs.end(), hashs2.begin(), hashs2.end());
}

BOOST_AUTO_TEST_SUITE_END()
}  // namespace test
}  // namespace bcos
