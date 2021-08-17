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

#include "libtable/TableFactory.h"
#include "../../../testutils/TestPromptFixture.h"
#include "Hash.h"
#include "interfaces/storage/TableInterface.h"
#include "libtable/Table.h"
#include "libutilities/ThreadPool.h"
#include "testutils/faker/FakeStorage.h"
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
struct TableFactoryFixture
{
    TableFactoryFixture()
    {
        hashImpl = make_shared<Header256Hash>();
        memoryStorage = make_shared<FakeStorage>();
        BOOST_TEST(memoryStorage != nullptr);
        tableFactory = make_shared<TableFactory>(memoryStorage, hashImpl, m_blockNumber);
        BOOST_TEST(tableFactory != nullptr);
    }

    ~TableFactoryFixture() {}
    bool createDefaultTable()
    {
        return tableFactory->createTable(testTableName, keyField, valueField);
    }
    std::shared_ptr<crypto::Hash> hashImpl = nullptr;
    std::shared_ptr<StorageInterface> memoryStorage = nullptr;
    protocol::BlockNumber m_blockNumber = 0;
    std::shared_ptr<TableFactory> tableFactory = nullptr;
    std::string testTableName = "t_test";
    std::string keyField = "key";
    std::string valueField = "value";
};
BOOST_FIXTURE_TEST_SUITE(TableFactoryTest, TableFactoryFixture)

BOOST_AUTO_TEST_CASE(constructor)
{
    auto threadPool = ThreadPool("a", 1);
    auto tf = std::make_shared<TableFactory>(memoryStorage, nullptr, 0);
}

BOOST_AUTO_TEST_CASE(create_Table)
{
    std::string tableName("t_test1");
    auto table = tableFactory->openTable(tableName);
    BOOST_TEST(table == nullptr);
    auto ret = tableFactory->createTable(tableName, keyField, valueField);
    BOOST_TEST(ret == true);
    table = tableFactory->openTable(tableName);
    BOOST_TEST(table != nullptr);
    ret = tableFactory->createTable(tableName, keyField, valueField);
    BOOST_TEST(ret == false);
}

BOOST_AUTO_TEST_CASE(rollback)
{
    auto ret = createDefaultTable();
    BOOST_TEST(ret == true);
    auto table = tableFactory->openTable(testTableName);
    table->remove("name");
    auto entry = table->newEntry();
    entry->setField("key", "name");
    entry->setField("value", "Lili");
    table->setRow("name", entry);
    entry = table->getRow("name");
    BOOST_TEST(entry != nullptr);
    BOOST_TEST(table->dirty() == true);
    BOOST_TEST(entry->dirty() == true);
    BOOST_TEST(entry->getField("value") == "Lili");

    auto savePoint = tableFactory->savepoint();

    entry = table->newEntry();
    entry->setField("key", "id");
    entry->setField("value", "12345");
    table->setRow("id", entry);
    entry = table->getRow("id");
    BOOST_TEST(entry != nullptr);
    entry = table->getRow("name");
    BOOST_TEST(entry != nullptr);
    BOOST_TEST(table->dirty() == true);

    auto savePoint1 = tableFactory->savepoint();

    entry = table->newEntry();
    entry->setField("key", "balance");
    entry->setField("value", "500");
    table->setRow("balance", entry);

    entry = table->getRow("balance");
    BOOST_TEST(entry != nullptr);

    entry = table->getRow("name");
    BOOST_TEST(entry != nullptr);
    BOOST_TEST(table->dirty() == true);

    auto savePoint2 = tableFactory->savepoint();

    table->remove("name");
    entry = table->getRow("name");
    BOOST_TEST(entry == nullptr);

    tableFactory->rollback(savePoint2);
    entry = table->getRow("name");
    BOOST_TEST(entry != nullptr);
    BOOST_TEST(table->dirty() == true);

    tableFactory->rollback(savePoint1);
    entry = table->getRow("name");
    BOOST_TEST(entry != nullptr);
    entry = table->getRow("balance");
    BOOST_TEST(entry == nullptr);
    BOOST_TEST(table->dirty() == true);

    // auto dbHash0 = tableFactory->hash();
    tableFactory->rollback(savePoint);
    entry = table->getRow("name");
    BOOST_TEST(entry != nullptr);
    entry = table->getRow("balance");
    BOOST_TEST(entry == nullptr);
    entry = table->getRow("id");
    BOOST_TEST(entry == nullptr);
    BOOST_TEST(table->dirty() == true);

    tableFactory->commit();
    // TODO: add some ut, setRow remove rollback getRow setRow, setRow setRow remove rollback getRow
}

BOOST_AUTO_TEST_CASE(rollback2)
{
    auto hash0 = tableFactory->hash();
    auto savePoint0 = tableFactory->savepoint();
    auto ret = createDefaultTable();
    BOOST_TEST(ret == true);
    auto table = tableFactory->openTable(testTableName);
    table->remove("name");
    auto entry = table->newEntry();
    entry->setField("key", "name");
    entry->setField("value", "Lili");
    table->setRow("name", entry);
    entry = table->getRow("name");
    BOOST_TEST(entry != nullptr);
    BOOST_TEST(table->dirty() == true);
    BOOST_TEST(entry->dirty() == true);
    BOOST_TEST(entry->getField("value") == "Lili");

    auto savePoint = tableFactory->savepoint();

    entry = table->newEntry();
    entry->setField("key", "id");
    entry->setField("value", "12345");
    table->setRow("id", entry);
    entry = table->getRow("id");
    BOOST_TEST(entry != nullptr);
    entry = table->getRow("name");
    BOOST_TEST(entry != nullptr);
    BOOST_TEST(table->dirty() == true);

    tableFactory->rollback(savePoint);
    entry = table->getRow("name");
    BOOST_TEST(entry != nullptr);
    entry = table->getRow("balance");
    BOOST_TEST(entry == nullptr);
    entry = table->getRow("id");
    BOOST_TEST(entry == nullptr);
    BOOST_TEST(table->dirty() == true);
    tableFactory->rollback(savePoint0);
    auto hash00 = tableFactory->hash();
    BOOST_TEST(hash00 == hash0);
    table = tableFactory->openTable(testTableName);
    BOOST_TEST(table == nullptr);
}

BOOST_AUTO_TEST_CASE(hash)
{
    auto ret = createDefaultTable();
    BOOST_TEST(ret == true);
    auto table = tableFactory->openTable(testTableName);
    auto entry = table->newEntry();
    entry->setField("key", "name");
    entry->setField("value", "Lili");
    ret = table->setRow("name", entry);
    BOOST_TEST(ret == true);
    entry = table->getRow("name");
    BOOST_TEST(entry != nullptr);
    BOOST_TEST(table->dirty() == true);
    auto dbHash0 = tableFactory->hash();
    auto data0 = tableFactory->exportData(m_blockNumber);
    auto tableFactory0 = make_shared<TableFactory>(memoryStorage, hashImpl, m_blockNumber);
    tableFactory0->importData(data0.first, data0.second);
    BOOST_TEST(dbHash0 == tableFactory0->hash());
    tableFactory0 = make_shared<TableFactory>(memoryStorage, hashImpl, m_blockNumber);
    tableFactory0->importData(data0.first, data0.second, false);
    BOOST_TEST(crypto::HashType() == tableFactory0->hash());

    entry = table->newEntry();
    entry->setField("key", "id");
    entry->setField("value", "12345");
    ret = table->setRow("id", entry);
    BOOST_TEST(ret == true);
    entry = table->getRow("id");
    BOOST_TEST(entry != nullptr);
    entry = table->getRow("name");
    BOOST_TEST(entry != nullptr);
    BOOST_TEST(table->dirty() == true);
    auto keys = table->getPrimaryKeys(nullptr);
    BOOST_TEST(keys.size() == 2);
    auto entries = table->getRows(keys);
    BOOST_TEST(entries.size() == 2);
    auto data1 = tableFactory->exportData(m_blockNumber);
    auto dbHash1 = tableFactory->hash();
    BOOST_TEST(dbHash1 != dbHash0);
    auto tableFactory1 = make_shared<TableFactory>(memoryStorage, hashImpl, m_blockNumber);
    tableFactory1->importData(data1.first, data1.second);
    BOOST_TEST(dbHash1 == tableFactory1->hash());
    tableFactory1 = make_shared<TableFactory>(memoryStorage, hashImpl, m_blockNumber);
    tableFactory1->importData(data0.first, data0.second, false);
    BOOST_TEST(crypto::HashType() == tableFactory1->hash());

    auto savePoint = tableFactory->savepoint();
    ret = table->remove("id");
    BOOST_TEST(ret == true);
    entry = table->getRow("id");
    BOOST_TEST(entry == nullptr);
    auto data2 = tableFactory->exportData(m_blockNumber);
    auto dbHash2 = tableFactory->hash();
    auto tableFactory2 = make_shared<TableFactory>(memoryStorage, hashImpl, m_blockNumber);
    tableFactory2->importData(data2.first, data2.second);
    BOOST_TEST(dbHash2 == tableFactory2->hash());
    cout << LOG_KV("dbHash1", dbHash1) << LOG_KV("dbHash2", dbHash2);

    BOOST_TEST(dbHash1 != dbHash2);

    tableFactory->rollback(savePoint);
    entry = table->getRow("name");
    BOOST_TEST(entry != nullptr);
    entry = table->getRow("balance");
    BOOST_TEST(entry == nullptr);
    BOOST_TEST(table->dirty() == true);

    auto data3 = tableFactory->exportData(m_blockNumber);
    auto dbHash3 = tableFactory->hash();
    cout << LOG_KV("dbHash3", dbHash3) << LOG_KV("dbHash1", dbHash1);
    BOOST_TEST(dbHash3 == dbHash1);
    auto tableFactory3 = make_shared<TableFactory>(memoryStorage, hashImpl, m_blockNumber);
    tableFactory3->importData(data3.first, data3.second);
    BOOST_TEST(dbHash3 == tableFactory3->hash());
    tableFactory->commit();
    tableFactory = make_shared<TableFactory>(memoryStorage, hashImpl, 1);
    table = tableFactory->openTable(testTableName);

    // getPrimaryKeys and getRows
    entry = table->newEntry();
    entry->setField("key", "id");
    entry->setField("value", "12345");
    ret = table->setRow("id", entry);
    BOOST_TEST(ret == true);
    entry = table->getRow("name");
    entry->setField("value", "Wang");
    ret = table->setRow("name", entry);
    BOOST_TEST(ret == true);
    entry = table->newEntry();
    entry->setField("key", "balance");
    entry->setField("value", "12345");
    ret = table->setRow("balance", entry);
    BOOST_TEST(ret == true);
    BOOST_TEST(entry != nullptr);
    keys = table->getPrimaryKeys(nullptr);
    BOOST_TEST(keys.size() == 3);
    entries = table->getRows(keys);
    BOOST_TEST(entries.size() == 3);
    entry = table->getRow("name");
    BOOST_TEST(entry != nullptr);
    entry = table->getRow("balance");
    BOOST_TEST(entry != nullptr);
    entry = table->getRow("balance1");
    BOOST_TEST(entry == nullptr);
    ret = table->remove("name");
    BOOST_TEST(ret == true);
    entry = table->getRow("name");
    BOOST_TEST(entry == nullptr);
    keys = table->getPrimaryKeys(nullptr);
    BOOST_TEST(keys.size() == 2);
    entries = table->getRows(keys);
    BOOST_TEST(entries.size() == 2);
    ret = table->remove("id");
    BOOST_TEST(ret == true);
    entry = table->getRow("id");
    BOOST_TEST(entry == nullptr);
    keys = table->getPrimaryKeys(nullptr);
    BOOST_TEST(keys.size() == 1);
    entries = table->getRows(keys);
    BOOST_TEST(entries.size() == 1);
    tableFactory->asyncCommit([](Error::Ptr, size_t) {});
}

BOOST_AUTO_TEST_CASE(parallel_openTable)
{
#if 0
    tableFactory->createTable(testTableName, keyField, valueField);
    auto table = tableFactory->openTable(testTableName);
    auto threadID = tbb::this_tbb_thread::get_id();

    tbb::parallel_for(tbb::blocked_range<size_t>(0, 10), [&](const tbb::blocked_range<size_t>& _r) {
        if (tbb::this_tbb_thread::get_id() == threadID)
        {
            return;
        }

        auto i = _r.begin();

        auto entry = table->newEntry();
        // entry->setField("key", "balance");
        auto initBalance = std::to_string(500 + i);
        entry->setField("value", initBalance);
        auto key = std::to_string(i);
        auto entries = table->getRow(key);
        auto savepoint0 = tableFactory->savepoint();

        table->setRow(key, entry);

        entry = table->getRow(key);
        BOOST_TEST(entry->getField("value") == initBalance);

        tbb::this_tbb_thread::sleep(tbb::tick_count::interval_t((double)i / 100));
        auto savepoint1 = tableFactory->savepoint();
        BOOST_TEST(savepoint1 == savepoint0 + 1);

        entry = table->newEntry();
        entry->setField("value", std::to_string((i + 1) * 100));
        table->setRow(key, entry);
        entry = table->getRow(key);
        BOOST_TEST(entry->getField("value") == std::to_string((i + 1) * 100));

        tableFactory->rollback(savepoint1);
        entry = table->getRow(key);
        BOOST_TEST(entry->getField("value") == initBalance);

        tableFactory->rollback(savepoint0);
        entries = table->getRow(key);

        entry = table->newEntry();
        // entry->setField("key", "name");
        entry->setField("value", "Vita");
        table->setRow(key, entry);

        entries = table->getRow(key);

        auto savepoint2 = tableFactory->savepoint();

        table->remove(key);
        entries = table->getRow(key);

        tableFactory->rollback(savepoint2);
        entry = table->getRow(key);
        BOOST_TEST(entry->getStatus() == 0);
    });

    tbb::parallel_for(tbb::blocked_range<size_t>(0, 10), [&](const tbb::blocked_range<size_t>& _r) {
        if (tbb::this_tbb_thread::get_id() == threadID)
        {
            return;
        }

        auto i = _r.begin();
        auto entry = table->newEntry();
        auto initBalance = std::to_string(500 + i);
        entry->setField("value", initBalance);
        auto key = std::to_string(i + 10);
        entry = table->getRow(key);
        auto savepoint0 = tableFactory->savepoint();
        table->setRow(key, entry);
        auto savepoint1 = tableFactory->savepoint();
        BOOST_TEST(savepoint1 == savepoint0 + 1);

        entry = table->getRow(key);
        BOOST_TEST(entry->getField("value") == initBalance);

        tableFactory->rollback(savepoint0);
        entry = table->getRow(key);
        BOOST_TEST(entry != nullptr);
    });
    tableFactory->commit();
#endif
}

BOOST_AUTO_TEST_CASE(open_sysTables)
{
    auto table = tableFactory->openTable(SYS_TABLE);
    BOOST_TEST(table != nullptr);
}

BOOST_AUTO_TEST_CASE(openAndCommit)
{
    auto hashImpl2 = make_shared<Header256Hash>();
    auto memoryStorage2 = make_shared<FakeStorage>();
    auto tableFactory2 = make_shared<TableFactory>(memoryStorage2, hashImpl2, 10);

    for (int i = 10; i < 20; ++i)
    {
        BOOST_TEST(tableFactory2 != nullptr);

        std::string tableName = "testTable" + boost::lexical_cast<std::string>(i);
        auto key = "testKey" + boost::lexical_cast<std::string>(i);
        tableFactory2->createTable(tableName, "key", "value");
        auto table = tableFactory2->openTable(tableName);

        auto entry = table->newEntry();
        entry->setField("value", "hello world!");
        table->setRow(key, entry);

        std::promise<bool> getRow;
        table->asyncGetRow(key, [&](const Error::Ptr& error, const Entry::Ptr& result) {
            BOOST_CHECK_EQUAL(error, nullptr);
            BOOST_CHECK_EQUAL(result->count("value"), 1);
            BOOST_CHECK_EQUAL(result->getField("value"), "hello world!");

            getRow.set_value(true);
        });

        getRow.get_future().get();

        // auto data = tableFactory2->exportData(i);
        auto data = tableFactory2->exportData(9);

        auto tableFactory3 = make_shared<TableFactory>(memoryStorage2, hashImpl2, i + 1);
        // auto tableFactory3 = make_shared<TableFactory>(memoryStorage2, hashImpl2, 10 + 1); //
        // without commit, always current height + 1
        tableFactory3->importData(data.first, data.second, false);

        for (int j = i; j >= 10; --j)
        // for (int j = 10; j >= 10; --j)
        {
            std::string queryTableName = "testTable" + boost::lexical_cast<std::string>(j);
            auto queryKey = "testKey" + boost::lexical_cast<std::string>(j);
            auto table2 = tableFactory3->openTable(queryTableName);
            BOOST_CHECK_NE(table2, nullptr);
            auto entry2 = table2->getRow(queryKey);
            BOOST_CHECK_NE(entry2, nullptr);
            BOOST_CHECK_EQUAL(entry2->getField("value"), "hello world!");
        }
        tableFactory2 = tableFactory3;
    }
}

BOOST_AUTO_TEST_SUITE_END()
}  // namespace test
}  // namespace bcos
