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
 * @brief define the common service information
 * @file ServiceDesc.h
 * @author: yujiechen
 * @date: 2021-10-11
 */
#pragma once
#include <iostream>
#include <memory>
#include <string>
namespace bcos
{
namespace protocol
{
const std::string NATIVE_SERVANT_NAME = "NativeNodeObj";
const std::string NATIVE_SERVICE_NAME = "NativeNode";

const std::string LEDGER_SERVANT_NAME = "LedgerServiceObj";
const std::string LEDGER_SERVICE_NAME = "LedgerService";

const std::string SCHEDULER_SERVANT_NAME = "SchedulerServiceObj";
const std::string SCHEDULER_SERVICE_NAME = "SchedulerService";

const std::string FRONT_SERVANT_NAME = "FrontServiceObj";
const std::string FRONT_SERVICE_NAME = "FrontService";

const std::string GATEWAY_SERVANT_NAME = "GatewayServiceObj";
const std::string GATEWAY_SERVICE_NAME = "GatewayService";

const std::string TXPOOL_SERVANT_NAME = "TxPoolServiceObj";
const std::string TXPOOL_SERVICE_NAME = "TxPoolService";

const std::string CONSENSUS_SERVANT_NAME = "PBFTServiceObj";
const std::string CONSENSUS_SERVICE_NAME = "PBFTService";

const std::string RPC_SERVANT_NAME = "RpcServiceObj";
const std::string RPC_SERVICE_NAME = "RpcService";

const std::string GROUPMANAGER_SERVANT_NAME = "GroupManagerServiceObj";
const std::string GROUPMANAGER_SERVICE_NAME = "GroupManagerService";

inline std::string getApplicationName(
    std::string const& _chainID, std::string const& _groupID, std::string const& _nodeName)
{
    return (_chainID + _groupID + _nodeName);
}

inline std::string getPrxDesc(
    std::string const& _appName, const std::string& serviceName, std::string const& _objName)
{
    return _appName + "." + serviceName + "." + _objName;
}
}  // namespace protocol
}  // namespace bcos