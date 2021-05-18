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
 * @file TransactionSubmitResultFactoryImpl.h
 * @author: yujiechen
 * @date: 2021-05-08
 */
#pragma once
#include "interfaces/protocol/TransactionSubmitResultFactory.h"
#include "libprotocol/TransactionSubmitResultImpl.h"

namespace bcos
{
namespace protocol
{
class TransactionSubmitResultFactoryImpl : public TransactionSubmitResultFactory
{
public:
    using Ptr = std::shared_ptr<TransactionSubmitResultFactoryImpl>;
    TransactionSubmitResultFactoryImpl() = default;
    ~TransactionSubmitResultFactoryImpl() override {}

    TransactionSubmitResult::Ptr createTxSubmitResult(
        bcos::crypto::HashType const& _txHash, int32_t _status) override
    {
        return std::make_shared<TransactionSubmitResultImpl>(_txHash, (TransactionStatus)_status);
    }

    TransactionSubmitResult::Ptr createTxSubmitResult(TransactionReceipt::Ptr _receipt,
        bcos::crypto::HashType _txHash, int64_t _txIndex, bcos::crypto::HashType _blockHash,
        bytesConstRef _sender, bytesConstRef _to) override
    {
        return std::make_shared<TransactionSubmitResultImpl>(
            _receipt, _txHash, _txIndex, _blockHash, _sender, _to);
    }

    TransactionSubmitResult::Ptr createTxSubmitResult(TransactionReceipt::Ptr _receipt,
        Transaction::Ptr _tx, int64_t _txIndex, BlockHeader::Ptr _blockHeader) override
    {
        return std::make_shared<TransactionSubmitResultImpl>(_receipt, _tx, _txIndex, _blockHeader);
    }
};
}  // namespace protocol
}  // namespace bcos