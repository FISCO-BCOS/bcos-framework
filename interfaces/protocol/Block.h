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
#include "../../libcodec/scale/ScaleEncoderStream.h"
#include "../../libprotocol/ParallelMerkleProof.h"
#include "BlockHeader.h"
#include "Transaction.h"
#include "TransactionFactory.h"
#include "TransactionMetaData.h"
#include "TransactionReceipt.h"
#include "TransactionReceiptFactory.h"
#include <tbb/parallel_for.h>

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
    using ConstPtr = std::shared_ptr<Block const>;
    Block(
        TransactionFactory::Ptr _transactionFactory, TransactionReceiptFactory::Ptr _receiptFactory)
      : m_transactionFactory(_transactionFactory), m_receiptFactory(_receiptFactory)
    {}

    virtual ~Block() {}

    virtual void decode(bytesConstRef _data, bool _calculateHash, bool _checkSig) = 0;
    virtual void encode(bytes& _encodeData) const = 0;
    virtual bcos::crypto::HashType calculateTransactionRoot(bool _updateHeader)
    {
        auto txsRoot = bcos::crypto::HashType();
        // with no transactions
        if (transactionsSize() == 0)
        {
            updateTxsRootForHeader(_updateHeader, txsRoot);
            return txsRoot;
        }
        // hit the cache
        UpgradableGuard l(x_txsRootCache);
        if (m_txsRootCache != bcos::crypto::HashType())
        {
            updateTxsRootForHeader(_updateHeader, m_txsRootCache);
            return m_txsRootCache;
        }
        // miss the cache or the cache has been cleared
        std::vector<bytes> transactionsList;
        encodeToCalculateRoot(transactionsList, transactionsSize(),
            [this](size_t _index) { return transaction(_index)->hash(); });
        txsRoot = calculateMerkleProofRoot(m_transactionFactory->cryptoSuite(), transactionsList);
        UpgradeGuard ul(l);
        m_txsRootCache = txsRoot;
        updateTxsRootForHeader(_updateHeader, txsRoot);
        return txsRoot;
    }

    virtual bcos::crypto::HashType calculateReceiptRoot(bool _updateHeader)
    {
        auto receiptsRoot = bcos::crypto::HashType();
        // with no receipts
        if (receiptsSize() == 0)
        {
            updateReceiptRootForHeader(_updateHeader, receiptsRoot);
            return receiptsRoot;
        }
        // hit the cache
        UpgradableGuard l(x_receiptRootCache);
        if (m_receiptRootCache != bcos::crypto::HashType())
        {
            updateReceiptRootForHeader(_updateHeader, m_receiptRootCache);
            return m_receiptRootCache;
        }
        // miss the cache or the cache has been cleared
        std::vector<bytes> receiptsList;
        encodeToCalculateRoot(receiptsList, receiptsSize(),
            [this](size_t _index) { return receipt(_index)->hash(); });
        receiptsRoot = calculateMerkleProofRoot(m_receiptFactory->cryptoSuite(), receiptsList);
        UpgradeGuard ul(l);
        m_receiptRootCache = receiptsRoot;
        updateReceiptRootForHeader(_updateHeader, receiptsRoot);
        return receiptsRoot;
    }

    virtual int32_t version() const = 0;
    virtual void setVersion(int32_t _version) = 0;
    virtual BlockType blockType() const = 0;
    // blockHeader gets blockHeader
    virtual BlockHeader::Ptr blockHeader() = 0;
    // get transactions
    virtual Transaction::ConstPtr transaction(size_t _index) const = 0;
    // get receipts
    virtual TransactionReceipt::ConstPtr receipt(size_t _index) const = 0;
    // get transaction metaData
    virtual TransactionMetaData::ConstPtr transactionMetaData(size_t _index) const = 0;
    // get transaction hash
    virtual bcos::crypto::HashType const& transactionHash(size_t _index) const
    {
        auto txMetaData = transactionMetaData(_index);
        if (txMetaData)
        {
            return txMetaData->hash();
        }
        return m_emptyHash;
    }

    virtual void setBlockType(BlockType _blockType) = 0;
    // setBlockHeader sets blockHeader
    virtual void setBlockHeader(BlockHeader::Ptr _blockHeader) = 0;
    // set transactions
    virtual void setTransaction(size_t _index, Transaction::Ptr _transaction) = 0;
    virtual void appendTransaction(Transaction::Ptr _transaction) = 0;
    // set receipts
    virtual void setReceipt(size_t _index, TransactionReceipt::Ptr _receipt) = 0;
    virtual void appendReceipt(TransactionReceipt::Ptr _receipt) = 0;
    // set transaction metaData
    virtual void appendTransactionMetaData(TransactionMetaData::Ptr _txMetaData) = 0;

    virtual NonceListPtr nonces() const
    {
        auto nonceList = std::make_shared<NonceList>();
        if (transactionsSize() == 0)
        {
            return nonceList;
        }
        for (size_t i = 0; i < transactionsSize(); ++i)
        {
            nonceList->push_back(transaction(i)->nonce());
        }
        return nonceList;
    }

    // get transactions size
    virtual size_t transactionsSize() const = 0;
    virtual size_t transactionsMetaDataSize() const = 0;
    virtual size_t transactionsHashSize() const { return transactionsMetaDataSize(); }

    // get receipts size
    virtual size_t receiptsSize() const = 0;

    // for nonceList
    virtual void setNonceList(NonceList const& _nonceList) = 0;
    virtual void setNonceList(NonceList&& _nonceList) = 0;
    virtual NonceList const& nonceList() const = 0;

private:
    void updateTxsRootForHeader(bool _updateHeader, bcos::crypto::HashType const& _txsRoot)
    {
        if (!_updateHeader || !blockHeader())
        {
            return;
        }
        blockHeader()->setTxsRoot(_txsRoot);
    }

    void updateReceiptRootForHeader(bool _updateHeader, bcos::crypto::HashType const& _receiptsRoot)
    {
        if (!_updateHeader || !blockHeader())
        {
            return;
        }
        blockHeader()->setReceiptsRoot(_receiptsRoot);
    }

    void encodeToCalculateRoot(std::vector<bytes>& _encodedList, size_t _listSize,
        std::function<bcos::crypto::HashType(size_t _index)> _hashFunc)
    {
        _encodedList.resize(_listSize);
        tbb::parallel_for(
            tbb::blocked_range<size_t>(0, _listSize), [&](const tbb::blocked_range<size_t>& _r) {
                for (auto i = _r.begin(); i < _r.end(); ++i)
                {
                    bcos::codec::scale::ScaleEncoderStream stream;
                    stream << i;
                    bytes encodedData = stream.data();
                    auto hash = _hashFunc(i);
                    encodedData.insert(encodedData.end(), hash.begin(), hash.end());
                    _encodedList[i] = std::move(encodedData);
                }
            });
    }

protected:
    TransactionFactory::Ptr m_transactionFactory;
    TransactionReceiptFactory::Ptr m_receiptFactory;
    // caches
    mutable bcos::crypto::HashType m_txsRootCache;
    mutable SharedMutex x_txsRootCache;

    mutable bcos::crypto::HashType m_receiptRootCache;
    mutable SharedMutex x_receiptRootCache;

    bcos::crypto::HashType m_emptyHash = bcos::crypto::HashType();
};
using Blocks = std::vector<Block::Ptr>;
using BlocksPtr = std::shared_ptr<Blocks>;
}  // namespace protocol
}  // namespace bcos
