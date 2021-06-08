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
 * @brief Unit tests for the fakers
 * @file FakersTest.cpp
 */
#include "../../../testutils/TestPromptFixture.h"
#include <testutils/faker/FakeDispatcher.h>
#include <testutils/faker/FakeFrontService.h>
#include <testutils/faker/FakeLedger.h>
#include <testutils/faker/FakeSealer.h>
#include <testutils/faker/FakeStorage.h>
#include <testutils/faker/FakeTxPool.h>
#include <boost/test/unit_test.hpp>
#include <string>

namespace bcos
{
namespace test
{
BOOST_FIXTURE_TEST_SUITE(FakersTest, TestPromptFixture)

BOOST_AUTO_TEST_CASE(fakeDispatcherConstructor)
{
    std::make_shared<FakeDispatcher>();
}

BOOST_AUTO_TEST_CASE(fakeFrontServiceConstructor)
{
    std::make_shared<FakeFrontService>(nullptr);
}

BOOST_AUTO_TEST_CASE(fakeLedgerConstructor)
{
    std::make_shared<FakeLedger>();
}

BOOST_AUTO_TEST_CASE(fakeStorageConstructor)
{
    std::make_shared<FakeStorage>();
}

BOOST_AUTO_TEST_CASE(fakeSealerConstructor)
{
    std::make_shared<FakeSealer>();
}

BOOST_AUTO_TEST_CASE(fakeTxPoolConstructor)
{
    std::make_shared<FakeTxPool>();
}
BOOST_AUTO_TEST_SUITE_END()
}  // namespace test
}  // namespace bcos
