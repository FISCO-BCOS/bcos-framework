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
const std::string SYS_CONFIG_NAME = "/sys/status";
const std::string CRUD_NAME = "/sys/crud";
const std::string TABLE_FACTORY_NAME = "/sys/table_storage";
const std::string CONSENSUS_NAME = "/sys/consensus";
const std::string CNS_NAME = "/sys/cns";
const std::string PERMISSION_NAME = "/sys/permission";
const std::string PARALLEL_CONFIG_NAME = "/sys/parallel_config";
const std::string CONTRACT_LIFECYCLE_NAME = "/sys/contract_mgr";
const std::string KV_TABLE_FACTORY_NAME = "/sys/kv_storage";
const std::string CRYPTO_NAME = "/sys/crypto_tools";
const std::string DAG_TRANSFER_NAME = "/sys/dag_test";
const std::string DEPLOY_WASM_NAME = "/sys/deploy_wasm";
const std::string BFS_NAME = "/sys/bfs";

/// precompiled contract for solidity
const std::string SYS_CONFIG_ADDRESS = "0000000000000000000000000000000000001000";
const std::string TABLE_FACTORY_ADDRESS = "0000000000000000000000000000000000001001";
const std::string CRUD_ADDRESS = "0000000000000000000000000000000000001002";
const std::string CONSENSUS_ADDRESS = "0000000000000000000000000000000000001003";
const std::string CNS_ADDRESS = "0000000000000000000000000000000000001004";
const std::string PERMISSION_ADDRESS = "0000000000000000000000000000000000001005";
const std::string PARALLEL_CONFIG_ADDRESS = "0000000000000000000000000000000000001006";
const std::string CONTRACT_LIFECYCLE_ADDRESS = "0000000000000000000000000000000000001007";
const std::string CHAIN_GOVERNANCE_ADDRESS = "0000000000000000000000000000000000001008";
const std::string KV_TABLE_FACTORY_ADDRESS = "0000000000000000000000000000000000001009";
const std::string CRYPTO_ADDRESS = "000000000000000000000000000000000000100a";
const std::string WORKING_SEALER_MGR_ADDRESS = "000000000000000000000000000000000000100b";
const std::string DAG_TRANSFER_ADDRESS = "000000000000000000000000000000000000100c";
const std::string DEPLOY_WASM_ADDRESS = "000000000000000000000000000000000000100d";
const std::string BFS_ADDRESS = "000000000000000000000000000000000000100e";
}  // namespace precompiled
}  // namespace bcos
