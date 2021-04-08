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
 * @brief typedef for protocol module
 * @file ProtocolTypeDef.h
 * @author: yujiechen
 * @date: 2021-04-9
 */
#pragma once
#include <bcos-framework/interfaces/crypto/CommonType.h>
namespace bcos
{
namespace protocol
{
using BlockNumber = int64_t;
using BytesList = std::vector<std::shared_ptr<bytes>>;
using BytesListPtr = std::shared_ptr<BytesList>;
using ParentInfoList = std::vector<std::pair<BlockNumber, bcos::crypto::HashType>>;
using ParentInfoListPtr = std::shared_ptr<ParentInfoList>;
using SignatureList = std::vector<std::pair<int64_t, std::shared_ptr<bytes>>>;
using SignatureListPtr = std::shared_ptr<SignatureList>;
int64_t constexpr InvalidSealerIndex = INT64_MAX;
}  // namespace protocol
}  // namespace bcos