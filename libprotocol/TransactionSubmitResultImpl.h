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
 * @file TransactionSubmitResultImpl.h
 * @author: yujiechen
 * @date: 2021-04-07
 */
#pragma once
#include "../interfaces/protocol/TransactionSubmitResult.h"
namespace bcos
{
namespace protocol
{
class TransactionSubmitResultImpl : public TransactionSubmitResult
{
public:
    using Ptr = std::shared_ptr<TransactionSubmitResultImpl>;
    TransactionSubmitResultImpl(TransactionReceipt::Ptr _receipt, bcos::crypto::HashType _txHash,
        int64_t _transactionIndex, bcos::crypto::HashType _blockHash, BlockNumber _blockNumber,
        bytesConstRef _sender, bytesConstRef _to)
      : m_receipt(_receipt),
        m_txHash(_txHash),
        m_transactionIndex(_transactionIndex),
        m_blockHash(_blockHash),
        m_blockNumber(_blockNumber),
        m_sender(_sender),
        m_to(_to)
    {}

    TransactionSubmitResultImpl(TransactionReceipt::Ptr _receipt, Transaction::Ptr _tx,
        int64_t _transactionIndex, BlockHeader::Ptr _blockHeader)
      : TransactionSubmitResultImpl(_receipt, _tx->hash(), _transactionIndex, _blockHeader->hash(),
            _blockHeader->number(), _tx->sender(), _tx->to())
    {}

    explicit TransactionSubmitResultImpl(TransactionStatus _status) : m_status((uint32_t)_status) {}
    virtual ~TransactionSubmitResultImpl() {}

    // get transaction status
    uint32_t status() const override { return m_status; }
    // get transaction receipt
    TransactionReceipt::Ptr receipt() const override { return m_receipt; }
    // get transactionHash
    bcos::crypto::HashType const& txHash() const override { return m_txHash; }
    // get blockHash
    bcos::crypto::HashType const& blockHash() const override { return m_blockHash; }
    // get blockNumber
    BlockNumber blockNumber() const override { return m_blockNumber; }
    // the sender
    bytesConstRef from() const override { return m_sender; }
    // to
    bytesConstRef to() const override { return m_to; }
    // txIndex
    int64_t transactionIndex() const override { return m_transactionIndex; }

private:
    uint32_t m_status = (uint32_t)TransactionStatus::None;
    TransactionReceipt::Ptr m_receipt;
    bcos::crypto::HashType m_txHash;
    int64_t m_transactionIndex;
    bcos::crypto::HashType m_blockHash;
    BlockNumber m_blockNumber;
    bytesConstRef m_sender;
    bytesConstRef m_to;
};
}  // namespace protocol
}  // namespace bcos