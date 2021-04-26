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
#include "BlockHeader.h"
#include "Transaction.h"
#include "TransactionReceipt.h"

namespace bcos
{
namespace protocol
{

using HashList = std::vector<bcos::crypto::HashType>;
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
    virtual bcos::crypto::HashType calculateTransactionRoot(bool _updateHeader) const = 0;
    virtual bcos::crypto::HashType calculateReceiptRoot(bool _updateHeader) const = 0;

    virtual int32_t version() const = 0;
    virtual void setVersion(int32_t _version) = 0;
    virtual BlockType blockType() const = 0;
    // blockHeader gets blockHeader
    virtual BlockHeader::Ptr blockHeader() = 0;
<<<<<<< HEAD
    // transactions get all transactions
    virtual TransactionsConstPtr transactions() const = 0;
    // transaction gets a transaction
    virtual Transaction::ConstPtr transaction(size_t _index) const = 0;
    // receipts get all receipts
    virtual ReceiptsConstPtr receipts() const = 0;
    // receipt gets a receipt
    virtual TransactionReceipt::ConstPtr receipt(size_t _index) const = 0;
    // transactionsHash get all transactions' hash
    virtual HashListConstPtr transactionsHash() const = 0;
    // transactionHash gets a transaction's hash
    virtual bcos::crypto::HashType const& transactionHash(size_t _index) const = 0;
    // receiptsHash get all receipts' hash
    virtual HashListConstPtr receiptsHash() const = 0;
    // receiptHash gets a receipt's hash
=======
    // get transactions
    virtual Transaction::ConstPtr transaction(size_t _index) const = 0;
    // get receipts
    virtual TransactionReceipt::ConstPtr receipt(size_t _index) const = 0;
    // get transaction hash
    virtual bcos::crypto::HashType const& transactionHash(size_t _index) const = 0;
    // get receipt hash
>>>>>>> 042360c885be987cedeecadf90bf33ddd695e195
    virtual bcos::crypto::HashType const& receiptHash(size_t _index) const = 0;

    virtual void setBlockType(BlockType _blockType) = 0;
    // setBlockHeader sets blockHeader
    virtual void setBlockHeader(BlockHeader::Ptr _blockHeader) = 0;
    // set transactions
    virtual void setTransaction(size_t _index, Transaction::Ptr _transaction) = 0;
    virtual void appendTransaction(Transaction::Ptr _transaction) = 0;
    // set receipts
    virtual void setReceipt(size_t _index, TransactionReceipt::Ptr _receipt) = 0;
    virtual void appendReceipt(TransactionReceipt::Ptr _receipt) = 0;
    // set transaction hash
    virtual void setTransactionHash(size_t _index, bcos::crypto::HashType const& _txHash) = 0;
    virtual void appendTransactionHash(bcos::crypto::HashType const& _txHash) = 0;
    // set receipt hash
    virtual void setReceiptHash(size_t _index, bcos::crypto::HashType const& _receptHash) = 0;
    virtual void appendReceiptHash(bcos::crypto::HashType const& _receiptHash) = 0;
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
using Blocks = std::vector<Block::Ptr>;
using BlocksPtr = std::shared_ptr<Blocks>;
}  // namespace protocol
}  // namespace bcos
