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
#include "../../interfaces/crypto/CryptoSuite.h"
#include "../../interfaces/protocol/Block.h"
#include "../../interfaces/protocol/BlockHeaderFactory.h"
#include "libprotocol/bcos-proto/Block.pb.h"
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
      : Block(_transactionFactory, _receiptFactory),
        m_blockHeaderFactory(_blockHeaderFactory),
        m_pbRawBlock(std::make_shared<PBRawBlock>()),
        m_transactions(std::make_shared<Transactions>()),
        m_receipts(std::make_shared<Receipts>()),
        m_transactionsHash(std::make_shared<HashList>()),
        m_nonceList(std::make_shared<NonceList>())
    {
        assert(m_blockHeaderFactory);
        assert(m_transactionFactory);
        assert(m_receiptFactory);
    }

    PBBlock(BlockHeaderFactory::Ptr _blockHeaderFactory,
        TransactionFactory::Ptr _transactionFactory, TransactionReceiptFactory::Ptr _receiptFactory,
        bytesConstRef _data, bool _calculateHash, bool _checkSig)
      : PBBlock(_blockHeaderFactory, _transactionFactory, _receiptFactory)
    {
        decode(_data, _calculateHash, _checkSig);
    }

    PBBlock(BlockHeaderFactory::Ptr _blockHeaderFactory,
        TransactionFactory::Ptr _transactionFactory, TransactionReceiptFactory::Ptr _receiptFactory,
        bytes const& _data, bool _calculateHash, bool _checkSig)
      : PBBlock(_blockHeaderFactory, _transactionFactory, _receiptFactory, ref(_data),
            _calculateHash, _checkSig)
    {}

    ~PBBlock() override {}

    void decode(bytesConstRef _data, bool _calculateHash, bool _checkSig) override;
    void encode(bytes& _encodeData) const override;

    // getNonces of the current block
    Transaction::ConstPtr transaction(size_t _index) const override;
    bcos::crypto::HashType const& transactionHash(size_t _index) const override;
    TransactionReceipt::ConstPtr receipt(size_t _index) const override;

    int32_t version() const override { return m_pbRawBlock->version(); }

    void setVersion(int32_t _version) override { m_pbRawBlock->set_version(_version); }

    BlockType blockType() const override { return (BlockType)m_pbRawBlock->type(); }
    // get blockHeader
    BlockHeader::Ptr blockHeader() override { return m_blockHeader; }
    // get transactions
    TransactionsConstPtr transactions() const { return m_transactions; }  // removed
    // get receipts
    ReceiptsConstPtr receipts() const { return m_receipts; }  // removed
    // get transaction hash
    HashListConstPtr transactionsHash() const { return m_transactionsHash; }  // removed

    void setBlockType(BlockType _blockType) override
    {
        m_pbRawBlock->set_type((int32_t)_blockType);
    }
    // set blockHeader
    void setBlockHeader(BlockHeader::Ptr _blockHeader) override { m_blockHeader = _blockHeader; }
    // set transactions
    void setTransactions(TransactionsPtr _transactions)  // removed
    {
        m_transactions = _transactions;
        clearTransactionsCache();
    }
    // Note: the caller must ensure the allocated transactions size
    void setTransaction(size_t _index, Transaction::Ptr _transaction) override
    {
        if (m_transactions->size() <= _index)
        {
            m_transactions->resize(_index + 1);
        }
        (*m_transactions)[_index] = _transaction;
        clearTransactionsCache();
    }
    void appendTransaction(Transaction::Ptr _transaction) override
    {
        m_transactions->push_back(_transaction);
        clearTransactionsCache();
    }
    // set receipts
    void setReceipts(ReceiptsPtr _receipts)  // removed
    {
        m_receipts = _receipts;
        // clear the cache
        clearReceiptsCache();
    }
    // Note: the caller must ensure the allocated receipts size
    void setReceipt(size_t _index, TransactionReceipt::Ptr _receipt) override
    {
        if (m_receipts->size() <= _index)
        {
            m_receipts->resize(_index + 1);
        }
        (*m_receipts)[_index] = _receipt;
        clearReceiptsCache();
    }

    void appendReceipt(TransactionReceipt::Ptr _receipt) override
    {
        m_receipts->push_back(_receipt);
        clearReceiptsCache();
    }
    // set transaction hash
    void setTransactionsHash(HashListPtr _transactionsHash)  // removed
    {
        m_transactionsHash = _transactionsHash;
        clearTransactionsHashCache();
    }
    void appendTransactionHash(bcos::crypto::HashType const& _txHash) override
    {
        m_transactionsHash->push_back(_txHash);
        clearTransactionsHashCache();
    }

    // get transactions size
    size_t transactionsSize() const override { return m_transactions->size(); }
    // get receipts size
    size_t receiptsSize() const override { return m_receipts->size(); }

    size_t transactionsHashSize() const override { return m_transactionsHash->size(); }
    void setNonceList(NonceList const& _nonceList) override
    {
        *m_nonceList = _nonceList;
        m_pbRawBlock->clear_noncelist();
    }

    void setNonceList(NonceList&& _nonceList) override
    {
        *m_nonceList = std::move(_nonceList);
        m_pbRawBlock->clear_noncelist();
    }
    NonceList const& nonceList() const override { return *m_nonceList; }

private:
    void decodeTransactions(bool _calculateHash, bool _checkSig);
    void decodeReceipts(bool _calculateHash);
    void decodeTxsHashList();
    void decodeNonceList();

    void encodeTransactions() const;
    void encodeReceipts() const;
    void encodeTransactionsHash() const;
    void encodeNonceList() const;

    void clearTransactionsCache()
    {
        m_pbRawBlock->clear_transactions();
        WriteGuard l(x_txsRootCache);
        m_txsRootCache = bcos::crypto::HashType();
    }
    void clearReceiptsCache()
    {
        m_pbRawBlock->clear_receipts();
        WriteGuard l(x_receiptRootCache);
        m_receiptRootCache = bcos::crypto::HashType();
    }

    void clearTransactionsHashCache() { m_pbRawBlock->clear_transactionshash(); }

private:
    BlockHeaderFactory::Ptr m_blockHeaderFactory;
    std::shared_ptr<PBRawBlock> m_pbRawBlock;
    BlockHeader::Ptr m_blockHeader;
    TransactionsPtr m_transactions;
    ReceiptsPtr m_receipts;
    HashListPtr m_transactionsHash;
    NonceListPtr m_nonceList;
};
}  // namespace protocol
}  // namespace bcos