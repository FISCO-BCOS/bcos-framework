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

#include "interfaces/storage/Storage.h"
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
BOOST_FIXTURE_TEST_SUITE(StorageInterface, TestPromptFixture)

BOOST_AUTO_TEST_CASE(testStorageInterface)
{
    Storage::Ptr s = nullptr;
    DB::Ptr d = nullptr;
    Table::Ptr t = nullptr;
}

BOOST_AUTO_TEST_SUITE_END()
}  // namespace test
}  // namespace bcos
