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
 * @brief Unit tests for the Entry
 * @file Entry.cpp
 */

#include "../../../testutils/TestPromptFixture.h"
#include "libtable/Table.h"
#include "libtable/TableStorage.h"
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <string>

using namespace std;
using namespace bcos;
using namespace bcos::storage;
namespace bcos
{
namespace test
{
struct EntryFixture
{
    EntryFixture() { tableInfo = std::make_shared<TableInfo>("testTable", "key", "key2,value"); }

    ~EntryFixture() {}

    std::shared_ptr<TableInfo> tableInfo;
};
BOOST_FIXTURE_TEST_SUITE(EntryTest, EntryFixture)

BOOST_AUTO_TEST_CASE(copyFrom)
{
    auto entry1 = std::make_shared<Entry>(tableInfo);
    auto entry2 = std::make_shared<Entry>(tableInfo);
    BOOST_CHECK_EQUAL(entry1->dirty(), false);
    entry1->setField("key2", "value");
    BOOST_TEST(entry1->dirty() == true);
    BOOST_TEST(entry1->capacityOfHashField() == 5);

    *entry2 = *entry1;
    BOOST_TEST(entry1->refCount() == 2);
    BOOST_TEST(entry2->refCount() == 2);

    {
        auto entry3 = Entry(*entry1);

        BOOST_CHECK_EQUAL(entry3.refCount(), 3);
        BOOST_CHECK_EQUAL(entry2->refCount(), 3);
        BOOST_CHECK_EQUAL(entry1->refCount(), 3);

        entry3.setField("value", "i am key2");
        BOOST_CHECK_EQUAL(entry3.refCount(), 1);
        BOOST_CHECK_EQUAL(entry2->refCount(), 2);
        BOOST_CHECK_EQUAL(entry1->refCount(), 2);

        auto entry4(std::move(entry3));
        BOOST_CHECK_EQUAL(entry4.refCount(), 1);
        BOOST_CHECK_EQUAL(entry3.refCount(), 0);

        auto entry5(*entry2);
        BOOST_CHECK_EQUAL(entry5.refCount(), 3);
        BOOST_CHECK_EQUAL(entry2->refCount(), 3);
        BOOST_CHECK_EQUAL(entry1->refCount(), 3);

        auto entry6(std::move(entry5));
        BOOST_CHECK_EQUAL(entry6.refCount(), 3);
        BOOST_CHECK_EQUAL(entry5.refCount(), 0);
        BOOST_CHECK_EQUAL(entry2->refCount(), 3);
        BOOST_CHECK_EQUAL(entry1->refCount(), 3);
    }

    BOOST_TEST(entry2->getField("key2") == "value");

    entry2->setField("key2", "value2");

    BOOST_TEST(entry2->getField("key2") == "value2");
    BOOST_TEST(entry1->getField("key2") == "value");
    BOOST_TEST(entry1->getField("key2") == "value");
    BOOST_TEST(entry1->getField("value") == "");
    BOOST_TEST(entry1->getField("value") == "");

    entry2->setField("key2", "value3");
    BOOST_TEST(entry2->capacityOfHashField() == 6);
    BOOST_TEST(entry2->getField("key2") == "value3");
    BOOST_TEST(entry1->refCount() == 1);
    BOOST_TEST(entry2->refCount() == 1);
    *entry2 = *entry2;
    BOOST_TEST(entry2->dirty() == true);
    entry2->setDirty(false);
    BOOST_TEST(entry2->dirty() == false);
    auto key2 = "value";
    // test setField lValue and rValue
    entry2->setField(key2, string("value2"));
    BOOST_TEST(entry2->dirty() == true);
    BOOST_TEST(entry2->refCount() == 1);
    BOOST_TEST(entry2->capacityOfHashField() == 12);
    auto value2 = "value2";
    entry2->setField(key2, value2);
}

BOOST_AUTO_TEST_CASE(functions)
{
    auto entry = std::make_shared<Entry>(tableInfo);
    BOOST_TEST(entry->dirty() == false);
    entry->setNum(1);
    BOOST_TEST(entry->num() == 1);
    BOOST_TEST(entry->status() == Entry::Status::NORMAL);
    entry->setStatus(Entry::Status::DELETED);
    BOOST_TEST(entry->status() == Entry::Status::DELETED);
    BOOST_TEST(entry->dirty() == true);
    BOOST_TEST(entry->rollbacked() == false);
    entry->setRollbacked(true);
    BOOST_TEST(entry->rollbacked() == true);
    BOOST_TEST(entry->dirty() == true);
}

BOOST_AUTO_TEST_SUITE_END()
}  // namespace test
}  // namespace bcos
