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
 * @file LedgerTypeDef.h
 * @author: kyonRay
 * @date 2021-04-30
 */

#pragma once

namespace bcos
{
namespace ledger
{
using MerkleProof = std::vector<std::pair<std::vector<std::string>, std::vector<std::string> > >;
using MerkleProofPtr = std::shared_ptr<const MerkleProof>;

// get block flag
static const int32_t FULL_BLOCK = 0xFFFF;
static const int32_t HEADER = 0x0008;
static const int32_t TRANSACTIONS = 0x0004;
static const int32_t RECEIPTS = 0x0002;

// get system config key
static const char* const SYSTEM_KEY_TX_COUNT_LIMIT = "tx_count_limit";
static const char* const SYSTEM_KEY_CONSENSUS_TIMEOUT = "consensus_timeout";

// get consensus node list type
static const char* const CONSENSUS_SEALER = "consensus_sealer";
static const char* const CONSENSUS_OBSERVER = "consensus_observer";

}  // namespace ledger
}  // namespace bcos
