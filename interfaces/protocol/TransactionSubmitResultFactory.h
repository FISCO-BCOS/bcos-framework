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
 * @file TransactionSubmitResultFactory.h
 * @author: yujiechen
 * @date: 2021-05-08
 */
#pragma once
#include "BlockHeader.h"
#include "Transaction.h"
#include "TransactionReceipt.h"
#include "TransactionSubmitResult.h"

namespace bcos
{
namespace protocol
{
class TransactionSubmitResultFactory
{
public:
    using Ptr = std::shared_ptr<TransactionSubmitResultFactory>;
    TransactionSubmitResultFactory() = default;
    virtual ~TransactionSubmitResultFactory() {}

    virtual TransactionSubmitResult::Ptr createTxSubmitResult(
        BlockHeader::Ptr _blockHeader, bcos::crypto::HashType const& _txHash) = 0;
    virtual TransactionSubmitResult::Ptr createTxSubmitResult(
        bcos::crypto::HashType const& _txHash, int32_t _status) = 0;
};
}  // namespace protocol
}  // namespace bcos