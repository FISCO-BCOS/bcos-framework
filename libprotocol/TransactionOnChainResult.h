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
 * @file TransactionOnChainResult.h
 * @author: yujiechen
 * @date: 2021-04-07
 */
#pragma once
#include <bcos-framework/interfaces/protocol/BlockHeader.h>
#include <bcos-framework/interfaces/protocol/Transaction.h>
#include <bcos-framework/interfaces/protocol/TransactionReceipt.h>
#include <bcos-framework/libprotocol/TransactionStatus.h>
namespace bcos
{
namespace protocol
{
class TransactionOnChainResult
{
public:
    using Ptr = std::shared_ptr<TransactionOnChainResult>;
    TransactionOnChainResult(TransactionReceipt::Ptr _receipt, bcos::crypto::HashType _txHash,
        int64_t _transactionIndex, bcos::crypto::HashType _blockHash, BlockNumber _blockNumber,
        Address const& _sender, Address const& _to)
      : m_receipt(_receipt),
        m_txHash(_txHash),
        m_transactionIndex(_transactionIndex),
        m_blockHash(_blockHash),
        m_blockNumber(_blockNumber),
        m_sender(_sender),
        m_to(_to)
    {}

    TransactionOnChainResult(TransactionReceipt::Ptr _receipt, Transaction::Ptr _tx,
        int64_t _transactionIndex, BlockHeader::Ptr _blockHeader)
      : TransactionOnChainResult(_receipt, _tx->hash(), _transactionIndex, _blockHeader->hash(),
            _blockHeader->number(), _tx->sender(), _tx->to())
    {}

    explicit TransactionOnChainResult(TransactionStatus _status) : m_status(_status) {}
    virtual ~TransactionOnChainResult() {}

    // get transaction status
    virtual TransactionStatus status() const { return m_status; }
    // get transaction receipt
    virtual TransactionReceipt::Ptr receipt() const { return m_receipt; }
    // get transactionHash
    virtual bcos::crypto::HashType const& txHash() const { return m_txHash; }
    // get blockHash
    virtual bcos::crypto::HashType const& blockHash() const { return m_blockHash; }
    // get blockNumber
    virtual BlockNumber blockNumber() const { return m_blockNumber; }
    // the sender
    virtual Address const& from() const { return m_sender; }
    // to
    virtual Address const& to() const { return m_to; }
    // txIndex
    virtual int64_t transactionIndex() const { return m_transactionIndex; }

private:
    TransactionStatus m_status = TransactionStatus::None;
    TransactionReceipt::Ptr m_receipt;
    bcos::crypto::HashType m_txHash;
    int64_t m_transactionIndex;
    bcos::crypto::HashType m_blockHash;
    BlockNumber m_blockNumber;
    Address m_sender;
    Address m_to;
};

using TransactionOnChainResults = std::vector<TransactionOnChainResult::Ptr>;
using TransactionOnChainResultsPtr = std::shared_ptr<TransactionOnChainResults>;
}  // namespace protocol
}  // namespace bcos