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
 * @brief interface for Block
 * @file Block.h
 * @author: yujiechen
 * @date: 2021-03-23
 */
#pragma once
#include <bcos-framework/interfaces/protocol/BlockHeader.h>
#include <bcos-framework/interfaces/protocol/Transaction.h>
#include <bcos-framework/interfaces/protocol/TransactionReceipt.h>

namespace bcos
{
namespace protocol
{
using HashList = std::vector<h256>;
using HashListPtr = std::shared_ptr<HashList>;
using HashListConstPtr = std::shared_ptr<const HashList>;
using NonceList = std::vector<u256>;
using NonceListPtr = std::shared_ptr<NonceList>;
enum BlockType : int32_t
{
    CompleteBlock = 1,
    WithTransactionsHash = 2,
};
class Block
{
public:
    using Ptr = std::shared_ptr<Block>;
    Block() = default;
    virtual ~Block() {}

    virtual void decode(bytesConstRef _data, bool _calculateHash, bool _checkSig) = 0;
    virtual void encode(bytes& _encodeData) const = 0;

    virtual int32_t version() const = 0;
    virtual void setVersion(int32_t _version) = 0;
    virtual BlockType blockType() const = 0;
    // get blockHeader
    virtual BlockHeader::Ptr blockHeader() const = 0;
    // get transactions
    virtual TransactionsConstPtr transactions() = 0;
    virtual Transaction::ConstPtr transaction(size_t _index) = 0;
    // get receipts
    virtual ReceiptsConstPtr receipts() = 0;
    virtual TransactionReceipt::ConstPtr receipt(size_t _index) = 0;
    // get transaction hash
    virtual HashListConstPtr transactionsHash() = 0;
    virtual h256 const& transactionHash(size_t _index) = 0;
    // get receipt hash
    virtual HashListConstPtr receiptsHash() = 0;
    virtual h256 const& receiptHash(size_t _index) = 0;

    virtual void setBlockType(BlockType _blockType) = 0;
    // set blockHeader
    virtual void setBlockHeader(BlockHeader::Ptr _blockHeader) = 0;
    // set transactions
    virtual void setTransactions(TransactionsPtr _transactions) = 0;
    virtual void setTransaction(size_t _index, Transaction::Ptr _transaction) = 0;
    virtual void appendTransaction(Transaction::Ptr _transaction) = 0;
    // set receipts
    virtual void setReceipts(ReceiptsPtr _receipts) = 0;
    virtual void setReceipt(size_t _index, TransactionReceipt::Ptr _receipt) = 0;
    virtual void appendReceipt(TransactionReceipt::Ptr _receipt) = 0;
    // set transaction hash
    virtual void setTransactionsHash(HashListPtr _transactionsHash) = 0;
    virtual void setTransactionHash(size_t _index, h256 const& _txHash) = 0;
    virtual void appendTransactionHash(h256 const& _txHash) = 0;
    // set receipt hash
    virtual void setReceiptsHash(HashListPtr _receiptsHash) = 0;
    virtual void setReceiptHash(size_t _index, h256 const& _receptHash) = 0;
    virtual void appendReceiptHash(h256 const& _receiptHash) = 0;
    // getNonces of the current block
    virtual NonceListPtr nonces() = 0;
    // get transactions size
    virtual size_t transactionsSize() = 0;
    virtual size_t transactionsHashSize() = 0;
    // get receipts size
    virtual size_t receiptsSize() = 0;
    virtual size_t receiptsHashSize() = 0;

    // TODO: set DAG mutually exclusive parameters
};
}  // namespace protocol
}  // namespace bcos
