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
#include "interfaces/crypto/CryptoSuite.h"
#include "interfaces/protocol/Block.h"
#include "interfaces/protocol/BlockHeaderFactory.h"
#include "interfaces/protocol/TransactionFactory.h"
#include "interfaces/protocol/TransactionReceiptFactory.h"
#include "libcodec/scale/ScaleEncoderStream.h"
#include "libprotocol/bcos-proto/Block.pb.h"
#include <tbb/parallel_for.h>
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
        m_receiptsHash(std::make_shared<HashList>()),
        m_nonceList(std::make_shared<NonceList>())
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

    bcos::crypto::HashType calculateTransactionRoot(bool _updateHeader) const override;
    bcos::crypto::HashType calculateReceiptRoot(bool _updateHeader) const override;

    // getNonces of the current block
    NonceListPtr nonces() override;
    Transaction::ConstPtr transaction(size_t _index) const override;
    bcos::crypto::HashType const& transactionHash(size_t _index) const override;
    TransactionReceipt::ConstPtr receipt(size_t _index) const override;
    bcos::crypto::HashType const& receiptHash(size_t _index) const override;

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
    // get receipt hash
    HashListConstPtr receiptsHash() const { return m_receiptsHash; }  // removed

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
        (*m_receipts)[_index] = _receipt;
        clearReceiptsCache();
    }
    void appendReceipt(TransactionReceipt::Ptr _receipt) override
    {
        m_receipts->push_back(_receipt);
    }
    // set transaction hash
    void setTransactionsHash(HashListPtr _transactionsHash)  // removed
    {
        m_transactionsHash = _transactionsHash;
        clearTransactionsHashCache();
    }
    void setTransactionHash(size_t _index, bcos::crypto::HashType const& _txHash) override
    {
        (*m_transactionsHash)[_index] = _txHash;
        clearTransactionsHashCache();
    }
    void appendTransactionHash(bcos::crypto::HashType const& _txHash) override
    {
        m_transactionsHash->push_back(_txHash);
    }
    // set receipt hash
    void setReceiptsHash(HashListPtr _receiptsHash)  // removed
    {
        m_receiptsHash = _receiptsHash;
        clearReceiptsHashCache();
    }

    void setReceiptHash(size_t _index, bcos::crypto::HashType const& _receiptHash) override
    {
        (*m_receiptsHash)[_index] = _receiptHash;
        clearReceiptsHashCache();
    }
    void appendReceiptHash(bcos::crypto::HashType const& _receiptHash) override
    {
        m_receiptsHash->push_back(_receiptHash);
    }
    // get transactions size
    size_t transactionsSize() override { return m_transactions->size(); }
    // get receipts size
    size_t receiptsSize() override { return m_receipts->size(); }

    size_t transactionsHashSize() override { return m_transactionsHash->size(); }
    size_t receiptsHashSize() override { return m_receiptsHash->size(); }

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
    void decodeReceiptsHashList();
    void decodeNonceList();

    void encodeTransactions() const;
    void encodeReceipts() const;
    void encodeTransactionsHash() const;
    void encodeReceiptsHash() const;
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

    void clearReceiptsHashCache() { m_pbRawBlock->clear_receiptshash(); }
    void updateTxsRootForHeader(bool _updateHeader, bcos::crypto::HashType const& _txsRoot) const
    {
        if (!_updateHeader || !m_blockHeader)
        {
            return;
        }
        m_blockHeader->setTxsRoot(_txsRoot);
    }

    void updateReceiptRootForHeader(
        bool _updateHeader, bcos::crypto::HashType const& _receiptsRoot) const
    {
        if (!_updateHeader || !m_blockHeader)
        {
            return;
        }
        m_blockHeader->setReceiptRoot(_receiptsRoot);
    }

    template <typename T>
    void encodeToCalculateRoot(std::vector<bytes>& _encodedList, T _protocolDataList) const
    {
        auto protocolDataSize = _protocolDataList->size();
        _encodedList.resize(protocolDataSize);
        tbb::parallel_for(tbb::blocked_range<size_t>(0, protocolDataSize),
            [&](const tbb::blocked_range<size_t>& _r) {
                for (auto i = _r.begin(); i < _r.end(); ++i)
                {
                    bcos::codec::scale::ScaleEncoderStream stream;
                    stream << i;
                    bytes encodedData = stream.data();
                    auto hash = ((*_protocolDataList)[i])->hash();
                    encodedData.insert(encodedData.end(), hash.begin(), hash.end());
                    _encodedList[i] = std::move(encodedData);
                }
            });
    }

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
    NonceListPtr m_nonceList;

    // caches
    mutable bcos::crypto::HashType m_txsRootCache;
    mutable SharedMutex x_txsRootCache;

    mutable bcos::crypto::HashType m_receiptRootCache;
    mutable SharedMutex x_receiptRootCache;
};
}  // namespace protocol
}  // namespace bcos