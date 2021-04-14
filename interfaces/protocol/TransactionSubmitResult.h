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
 * @file TransactionSubmitResult.h
 * @author: yujiechen
 * @date: 2021-04-07
 */
#pragma once
#include "interfaces/protocol/BlockHeader.h"
#include "interfaces/protocol/Transaction.h"
#include "interfaces/protocol/TransactionReceipt.h"
namespace bcos
{
namespace protocol
{
class TransactionSubmitResult
{
public:
    using Ptr = std::shared_ptr<TransactionSubmitResult>;
    TransactionSubmitResult() = default;
    virtual ~TransactionSubmitResult() {}

    // get transaction status
    virtual uint32_t status() const = 0;
    // get transaction receipt
    virtual TransactionReceipt::Ptr receipt() const = 0;
    // get transactionHash
    virtual bcos::crypto::HashType const& txHash() const = 0;
    // get blockHash
    virtual bcos::crypto::HashType const& blockHash() const = 0;
    // get blockNumber
    virtual BlockNumber blockNumber() const = 0;
    // the sender
    virtual bytes const& from() const = 0;
    // to
    virtual bytes const& to() const = 0;
    // txIndex
    virtual int64_t transactionIndex() const = 0;
};

using TransactionSubmitResults = std::vector<TransactionSubmitResult::Ptr>;
using TransactionSubmitResultsPtr = std::shared_ptr<TransactionSubmitResults>;
}  // namespace protocol
}  // namespace bcos