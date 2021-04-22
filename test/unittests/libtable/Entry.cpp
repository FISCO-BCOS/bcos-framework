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
 * @brief Unit tests for the Base64
 * @file Base64.cpp
 */

#include "interfaces/storage/TableInterface.h"
#include "libtable/Table.h"
#include "libtable/TableFactory.h"
#include <bcos-test/libutils/TestPromptFixture.h>
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <string>

using namespace bcos;
using namespace bcos::storage;
namespace bcos
{
namespace test
{
struct EntryFixture
{
    EntryFixture() {}

    ~EntryFixture() {}
};
BOOST_FIXTURE_TEST_SUITE(EntryTest, EntryFixture)

BOOST_AUTO_TEST_CASE(copyFrom)
{
    auto entry1 = std::make_shared<Entry>();
    auto entry2 = std::make_shared<Entry>();
    BOOST_TEST(entry1->dirty() == false);
    BOOST_TEST(entry1->count("key") == false);
    BOOST_TEST(entry1->size() == 0);
    entry1->setField("key", "value");
    BOOST_TEST(entry1->count("key") == true);
    BOOST_TEST(entry1->size() == 1);
    BOOST_TEST(entry1->dirty() == true);
    BOOST_TEST(entry1->capacityOfHashField() == 8);

    entry2->copyFrom(entry1);
    BOOST_TEST(entry1->refCount() == 2);
    BOOST_TEST(entry2->refCount() == 2);

    BOOST_TEST(entry2->getField("key") == "value");

    entry2->setField("key", "value2");

    BOOST_TEST(entry2->getField("key") == "value2");
    BOOST_TEST(entry1->getField("key") == "value");
    BOOST_TEST(entry1->getFieldConst("key") == "value");
    BOOST_TEST(entry1->getFieldConst("key2") == "");
    BOOST_TEST(entry1->getField("key2") == "");
    if(entry1->find("key2") != entry1->end())
    {
        BOOST_TEST(false);
    }
    if(entry1->find("key") != entry1->begin())
    {
        BOOST_TEST(false);
    }
    entry2->setField("key", "value3");
    BOOST_TEST(entry2->getFieldConst("key") == "value3");
    BOOST_TEST(entry1->refCount() == 1);
    BOOST_TEST(entry2->refCount() == 1);
    entry2->copyFrom(entry2);
}

BOOST_AUTO_TEST_CASE(functions)
{
    auto entry = std::make_shared<Entry>();
    BOOST_TEST(entry->dirty() == false);
    entry->setNum(1);
    BOOST_TEST(entry->num() == 1);
    BOOST_TEST(entry->getStatus() == Entry::Status::NORMAL);
    entry->setStatus(Entry::Status::DELETED);
    BOOST_TEST(entry->getStatus() == Entry::Status::DELETED);
    BOOST_TEST(entry->dirty() == true);
    BOOST_TEST(entry->rollbacked() == false);
    entry->setRollbacked(true);
    BOOST_TEST(entry->rollbacked() == true);
    BOOST_TEST(entry->dirty() == true);
}

BOOST_AUTO_TEST_SUITE_END()
}  // namespace test
}  // namespace bcos
