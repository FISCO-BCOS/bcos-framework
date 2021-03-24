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
 * @brief protobuf implementation of Block
 * @file PBBlock.h
 * @author: yujiechen
 * @date: 2021-03-23
 */
#pragma once
#include <bcos-framework/interfaces/crypto/CryptoSuite.h>
#include <bcos-framework/interfaces/protocol/Block.h>
#include <bcos-framework/interfaces/protocol/BlockHeaderFactory.h>
#include <bcos-framework/interfaces/protocol/TransactionFactory.h>
#include <bcos-framework/interfaces/protocol/TransactionReceiptFactory.h>
#include <bcos-framework/libprotocol/bcos-proto/Block.pb.h>

namespace bcos
{
namespace protocol
{
class PBBlock : public Block
{
public:
    using Ptr = std::shared_ptr<PBBlock>;
    PBBlock(BlockHeaderFactory::Ptr _blockHeaderFactory,
        TransactionFactory::Ptr _transactionFactory, TransactionReceiptFactory::Ptr _receiptFactory)
      : m_blockHeaderFactory(_blockHeaderFactory),
        m_transactionFactory(_transactionFactory),
        m_receiptFactory(_receiptFactory),
        m_pbRawBlock(std::make_shared<PBRawBlock>()),
        m_transactions(std::make_shared<Transactions>()),
        m_receipts(std::make_shared<Receipts>()),
        m_transactionsHash(std::make_shared<HashList>()),
        m_receiptsHash(std::make_shared<HashList>())
    {
        assert(m_blockHeaderFactory);
        assert(m_transactionFactory);
        assert(m_receiptFactory);
    }

    PBBlock(BlockHeaderFactory::Ptr _blockHeaderFactory,
        TransactionFactory::Ptr _transactionFactory, TransactionReceiptFactory::Ptr _receiptFactory,
        bytes const& _data, bool _calculateHash, bool _checkSig)
      : PBBlock(_blockHeaderFactory, _transactionFactory, _receiptFactory)
    {
        decode(ref(_data), _calculateHash, _checkSig);
    }

    ~PBBlock() override {}

    void decode(bytesConstRef _data, bool _calculateHash, bool _checkSig) override;
    void encode(bytes& _encodeData) const override;
    // getNonces of the current block
    NonceListPtr nonces() override;
    Transaction::ConstPtr transaction(size_t _index) override;
    h256 const& transactionHash(size_t _index) override;
    TransactionReceipt::ConstPtr receipt(size_t _index) override;
    h256 const& receiptHash(size_t _index) override;

    int32_t version() const override { return m_pbRawBlock->version(); }

    void setVersion(int32_t _version) override { m_pbRawBlock->set_version(_version); }

    BlockType blockType() const override { return (BlockType)m_pbRawBlock->type(); }
    // get blockHeader
    BlockHeader::Ptr blockHeader() const override { return m_blockHeader; }
    // get transactions
    TransactionsConstPtr transactions() override { return m_transactions; }
    // get receipts
    ReceiptsConstPtr receipts() override { return m_receipts; }
    // get transaction hash
    HashListConstPtr transactionsHash() override { return m_transactionsHash; }
    // get receipt hash
    HashListConstPtr receiptsHash() override { return m_receiptsHash; }

    void setBlockType(BlockType _blockType) override
    {
        m_pbRawBlock->set_type((int32_t)_blockType);
    }
    // set blockHeader
    void setBlockHeader(BlockHeader::Ptr _blockHeader) override { m_blockHeader = _blockHeader; }
    // set transactions
    void setTransactions(TransactionsPtr _transactions) override
    {
        m_transactions = _transactions;
        clearTransactionsCache();
    }
    // Note: the caller must ensure the allocated transactions size
    void setTransaction(size_t _index, Transaction::Ptr _transaction) override
    {
        (*m_transactions)[_index] = _transaction;
        clearTransactionsCache();
    }
    void appendTransaction(Transaction::Ptr _transaction) override
    {
        m_transactions->push_back(_transaction);
        clearTransactionsCache();
    }
    // set receipts
    void setReceipts(ReceiptsPtr _receipts) override
    {
        m_receipts = _receipts;
        // clear the cache
        clearReceiptsCache();
    }
    // Note: the caller must ensure the allocated receipts size
    void setReceipt(size_t _index, TransactionReceipt::Ptr _receipt) override
    {
        (*m_receipts)[_index] = _receipt;
        clearReceiptsCache();
    }
    void appendReceipt(TransactionReceipt::Ptr _receipt) override
    {
        m_receipts->push_back(_receipt);
    }
    // set transaction hash
    void setTransactionsHash(HashListPtr _transactionsHash) override
    {
        m_transactionsHash = _transactionsHash;
        clearTransactionsHashCache();
    }
    void setTransactionHash(size_t _index, h256 const& _txHash) override
    {
        (*m_transactionsHash)[_index] = _txHash;
        clearTransactionsHashCache();
    }
    void appendTransactionHash(h256 const& _txHash) override
    {
        m_transactionsHash->push_back(_txHash);
    }
    // set receipt hash
    void setReceiptsHash(HashListPtr _receiptsHash) override
    {
        m_receiptsHash = _receiptsHash;
        clearReceiptsHashCache();
    }

    void setReceiptHash(size_t _index, h256 const& _receiptHash) override
    {
        (*m_receiptsHash)[_index] = _receiptHash;
        clearReceiptsHashCache();
    }
    void appendReceiptHash(h256 const& _receiptHash) override
    {
        m_receiptsHash->push_back(_receiptHash);
    }
    // get transactions size
    size_t transactionsSize() override { return m_transactions->size(); }
    // get receipts size
    size_t receiptsSize() override { return m_receipts->size(); }

    size_t transactionsHashSize() override { return m_transactionsHash->size(); }
    size_t receiptsHashSize() override { return m_receiptsHash->size(); }

private:
    void decodeTransactions(bool _calculateHash, bool _checkSig);
    void decodeReceipts(bool _calculateHash);
    void decodeTxsHashList();
    void decodeReceiptsHashList();

    void encodeTransactions() const;
    void encodeReceipts() const;
    void encodeTransactionsHash() const;
    void encodeReceiptsHash() const;

    void clearTransactionsCache() { m_pbRawBlock->clear_transactions(); }
    void clearReceiptsCache() { m_pbRawBlock->clear_receipts(); }

    void clearTransactionsHashCache() { m_pbRawBlock->clear_transactionshash(); }

    void clearReceiptsHashCache() { m_pbRawBlock->clear_receiptshash(); }

private:
    BlockHeaderFactory::Ptr m_blockHeaderFactory;
    TransactionFactory::Ptr m_transactionFactory;
    TransactionReceiptFactory::Ptr m_receiptFactory;
    std::shared_ptr<PBRawBlock> m_pbRawBlock;
    BlockHeader::Ptr m_blockHeader;
    TransactionsPtr m_transactions;
    ReceiptsPtr m_receipts;
    HashListPtr m_transactionsHash;
    HashListPtr m_receiptsHash;
};
}  // namespace protocol
}  // namespace bcos