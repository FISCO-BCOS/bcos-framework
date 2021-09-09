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
 * @brief Unit tests for the Dispatcher
 * @file DispatcherTest.cpp
 */

#include "interfaces/dispatcher/DispatcherInterface.h"
#include "interfaces/gateway/GatewayInterface.h"
#include "interfaces/multigroup/GroupManagerInterface.h"
#include "interfaces/rpc/RPCInterface.h"
#include <boost/test/unit_test.hpp>
#include <string>

using namespace std;
using namespace bcos;
using namespace bcos::dispatcher;

namespace bcos
{
namespace test
{
struct DispatcherFixture
{
    DispatcherFixture() {}

    ~DispatcherFixture() {}
};
BOOST_FIXTURE_TEST_SUITE(DispatcherTest, DispatcherFixture)

BOOST_AUTO_TEST_CASE(dispatcherConstructor)
{
    shared_ptr<DispatcherInterface> dp = nullptr;
}
BOOST_AUTO_TEST_CASE(groupMgrConstructor)
{
    std::shared_ptr<bcos::group::GroupManagerInterface> groupMgr = nullptr;
}
BOOST_AUTO_TEST_CASE(GatewayConstructor)
{
    std::shared_ptr<bcos::gateway::GatewayInterface> gateway = nullptr;
}
BOOST_AUTO_TEST_CASE(rpcConstructor)
{
    std::shared_ptr<bcos::rpc::RPCInterface> rpc = nullptr;
}
BOOST_AUTO_TEST_SUITE_END()
}  // namespace test
}  // namespace bcos
