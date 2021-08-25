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
namespace bcos
{
namespace test
{
struct TableFixture
{
    TableFixture()
    {
        hashImpl = make_shared<Header256Hash>();
        memoryStorage = make_shared<storage::TableStorage>();
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
    auto table = std::make_shared<Table>(nullptr, nullptr, nullptr, 0);
    auto tableFactory = std::make_shared<TableStorage>(memoryStorage, nullptr, 0);
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
    entry->setField("key", "name");
    entry->setField("value", "Lili");
    table->setRow("name", entry);
    auto tableinfo = table->tableInfo();
    BOOST_CHECK_EQUAL(tableinfo->name, tableName);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        valueField.begin(), valueField.end(), tableinfo->fields.begin(), tableinfo->fields.end());

    auto hash = tableFactory->tablesHash();
    BOOST_CHECK_EQUAL(hash.size(), 1);
    BOOST_CHECK_EQUAL(std::get<1>(hash[0]).size, 32);

    // BOOST_CHECK_EQUAL(tableFactory.ex)

    // auto data = table->dump(m_blockNumber);
    // auto hash = table->hash();
    // BOOST_TEST(data->size() == 1);
    entry = table->newEntry();
    entry->setField("key", "name2");
    entry->setField("value", "WW");
    table->setRow("name2", entry);
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
    entry->setField("key", "name");
    entry->setField("value", "Lili");
    entry->setField("invalid", "name");
    ret = table->setRow("name", entry);
    // BOOST_TEST(ret == false); // always success

    // check fields order of SYS_TABLE
    table = tableFactory->openTable(SYS_TABLE);
    BOOST_TEST(table != nullptr);
    BOOST_TEST(table->tableInfo()->fields.size() == 2);
    BOOST_TEST(table->tableInfo()->fields[0] == SYS_TABLE_KEY_FIELDS);
    BOOST_TEST(table->tableInfo()->fields[1] == SYS_TABLE_VALUE_FIELDS);
    BOOST_TEST(table->tableInfo()->key == SYS_TABLE_KEY);
}
BOOST_AUTO_TEST_SUITE_END()
}  // namespace test
}  // namespace bcos
