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
 * @file PrecompiledTypeDef.h
 * @author: kyonRay
 * @date 2021-06-22
 */

#pragma once
#include "../../libutilities/Common.h"

namespace bcos
{
namespace precompiled
{
/// precompiled contract path for wasm
const std::string SYS_CONFIG_NAME = "/bin/status";
const std::string CRUD_NAME = "/bin/crud";
const std::string CONSENSUS_NAME = "/bin/consensus";
const std::string CNS_NAME = "/bin/cns";
const std::string PERMISSION_NAME = "/bin/permission";
const std::string PARALLEL_CONFIG_NAME = "/bin/parallel_config";
const std::string KV_TABLE_FACTORY_NAME = "/bin/kv_storage";
const std::string DEPLOY_WASM_NAME = "/bin/deploy_wasm"

/// precompiled contract for solidity
const std::string SYS_CONFIG_ADDRESS = "0x1000";
const std::string TABLE_FACTORY_ADDRESS = "0x1001";
const std::string CRUD_ADDRESS = "0x1002";
const std::string CONSENSUS_ADDRESS = "0x1003";
const std::string CNS_ADDRESS = "0x1004";
const std::string PERMISSION_ADDRESS = "0x1005";
const std::string PARALLEL_CONFIG_ADDRESS = "0x1006";
const std::string CONTRACT_LIFECYCLE_ADDRESS = "0x1007";
const std::string CHAIN_GOVERNANCE_ADDRESS = "0x1008";
const std::string KV_TABLE_FACTORY_ADDRESS = "0x1009";
const std::string CRYPTO_ADDRESS = "0x100a";
const std::string WORKING_SEALER_MGR_ADDRESS = "0x100b";
const std::string DAG_TRANSFER_ADDRESS = "0x100c";
const std::string DEPLOY_WASM_ADDRESS = "0x100d"
}  // namespace precompiled
}  // namespace bcos
