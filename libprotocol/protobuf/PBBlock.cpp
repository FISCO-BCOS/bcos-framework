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
 * @file PBBlock.cpp
 * @author: yujiechen
 * @date: 2021-03-23
 */
#include "PBBlock.h"
#include <bcos-framework/libprotocol/Exceptions.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_invoke.h>

using namespace bcos;
using namespace bcos::protocol;

void PBBlock::decode(bytesConstRef _data, bool _calculateHash, bool _checkSig)
{
    if (!m_pbRawBlock->ParseFromArray(_data.data(), _data.size()))
    {
        BOOST_THROW_EXCEPTION(BlockDecodeException() << errinfo_comment(
                                  "decode block exception, blockData:" + *toHexString(_data)));
    }

    tbb::parallel_invoke(
        [this]() {
            // decode blockHeader
            auto blockHeaderData = m_pbRawBlock->mutable_header();
            if (blockHeaderData->size() > 0)
            {
                m_blockHeader = m_blockHeaderFactory->createBlockHeader(
                    bytesConstRef((byte const*)blockHeaderData->data(), blockHeaderData->size()));
            }
        },
        [this, _calculateHash, _checkSig]() {
            // decode transactions
            decodeTransactions(_calculateHash, _checkSig);
        },
        [this, _calculateHash]() {
            // decode receipts
            decodeReceipts(_calculateHash);
        },
        [this]() {
            // decode txsHashList
            decodeTxsHashList();
        },
        [this]() {
            // decode ReceiptHashList
            decodeReceiptsHashList();
        });
}

void PBBlock::encode(bytes& _encodedData) const
{
    tbb::parallel_invoke(
        [this]() {
            // encode blockHeader
            if (!m_blockHeader)
            {
                return;
            }
            auto encodedData = std::make_shared<bytes>();
            m_blockHeader->encode(*encodedData);
            m_pbRawBlock->set_header(encodedData->data(), encodedData->size());
        },
        [this]() {
            // encode transactions
            encodeTransactions();
        },
        [this]() {
            // encode receipts
            encodeReceipts();
        },
        [this]() {
            // encode transactions hash
            encodeTransactionsHash();
        },
        [this]() {
            // encode receipts hash
            encodeReceiptsHash();
        });
    auto blockLen = m_pbRawBlock->ByteSizeLong();
    _encodedData.resize(blockLen);
    if (!m_pbRawBlock->SerializeToArray(_encodedData.data(), blockLen))
    {
        BOOST_THROW_EXCEPTION(BlockEncodeException() << errinfo_comment("encode Block failed"));
    }
}

void PBBlock::decodeTxsHashList()
{
    auto txsHashNum = m_pbRawBlock->transactionshash_size();
    if (txsHashNum == 0)
    {
        return;
    }
    // decode the transactionsHash
    for (auto i = 0; i < txsHashNum; i++)
    {
        m_transactionsHash->push_back(h256((byte const*)m_pbRawBlock->transactionshash(i).data(),
            h256::ConstructorType::FromPointer));
    }
}

void PBBlock::decodeReceiptsHashList()
{
    auto receiptsHashNum = m_pbRawBlock->receiptshash_size();
    if (receiptsHashNum == 0)
    {
        return;
    }
    // decode the transactionsHash
    for (auto i = 0; i < receiptsHashNum; i++)
    {
        m_receiptsHash->push_back(h256(
            (byte const*)m_pbRawBlock->receiptshash(i).data(), h256::ConstructorType::FromPointer));
    }
}

void PBBlock::decodeTransactions(bool _calculateHash, bool _checkSig)
{
    // Does not contain transaction fields
    if (m_pbRawBlock->mutable_transactions()->size() == 0)
    {
        return;
    }
    // Parallel decode transaction
    m_transactions->clear();
    int txsNum = m_pbRawBlock->transactions_size();
    m_transactions->resize(txsNum);
    tbb::parallel_for(tbb::blocked_range<int>(0, txsNum), [&](const tbb::blocked_range<int>& _r) {
        for (auto i = _r.begin(); i < _r.end(); i++)
        {
            auto const& txData = m_pbRawBlock->transactions(i);
            (*m_transactions)[i] = m_transactionFactory->createTransaction(
                bytesConstRef((byte const*)txData.data(), txData.size()), _checkSig);
            if (_calculateHash)
            {
                ((*m_transactions)[i])->hash();
            }
        }
    });
}

void PBBlock::decodeReceipts(bool _calculateHash)
{
    // Does not contain receipts fields
    if (m_pbRawBlock->mutable_receipts()->size() == 0)
    {
        return;
    }
    // Parallel decode receipt
    m_receipts->clear();
    int receiptsNum = m_pbRawBlock->receipts_size();
    m_receipts->resize(receiptsNum);
    tbb::parallel_for(
        tbb::blocked_range<int>(0, receiptsNum), [&](const tbb::blocked_range<int>& _r) {
            for (auto i = _r.begin(); i < _r.end(); i++)
            {
                auto const& receiptData = m_pbRawBlock->receipts(i);
                (*m_receipts)[i] = m_receiptFactory->createReceipt(
                    bytesConstRef((byte const*)receiptData.data(), receiptData.size()));
                if (_calculateHash)
                {
                    ((*m_receipts)[i])->hash();
                }
            }
        });
}

void PBBlock::encodeTransactions() const
{
    auto txsNum = m_transactions->size();
    if (txsNum == 0)
    {
        return;
    }
    // hit the transaction cache
    if (m_pbRawBlock->transactions_size() > 0)
    {
        return;
    }
    // extend transactions
    for (size_t i = 0; i < txsNum; i++)
    {
        m_pbRawBlock->add_transactions();
    }
    // parallel encode transactions
    tbb::parallel_for(tbb::blocked_range<int>(0, txsNum), [&](const tbb::blocked_range<int>& _r) {
        for (auto i = _r.begin(); i < _r.end(); i++)
        {
            auto encodedData = std::make_shared<bytes>();
            (*m_transactions)[i]->encode(*encodedData);
            m_pbRawBlock->set_transactions(i, encodedData->data(), encodedData->size());
        }
    });
}
void PBBlock::encodeReceipts() const
{
    auto receiptsNum = m_receipts->size();
    if (receiptsNum == 0)
    {
        return;
    }
    // hit the receipts cache
    if (m_pbRawBlock->receipts_size() > 0)
    {
        return;
    }
    // extend transactions
    for (size_t i = 0; i < receiptsNum; i++)
    {
        m_pbRawBlock->add_receipts();
    }
    // parallel encode transactions
    tbb::parallel_for(
        tbb::blocked_range<int>(0, receiptsNum), [&](const tbb::blocked_range<int>& _r) {
            for (auto i = _r.begin(); i < _r.end(); i++)
            {
                auto encodedData = std::make_shared<bytes>();
                (*m_receipts)[i]->encode(*encodedData);
                m_pbRawBlock->set_receipts(i, encodedData->data(), encodedData->size());
            }
        });
}

void PBBlock::encodeTransactionsHash() const
{
    auto txsHashNum = m_transactionsHash->size();
    if (txsHashNum == 0)
    {
        return;
    }
    // hit the cache
    if (m_pbRawBlock->transactionshash_size() > 0)
    {
        return;
    }
    // extend transactionshash
    for (size_t i = 0; i < txsHashNum; i++)
    {
        m_pbRawBlock->add_transactionshash();
    }
    int index = 0;
    for (auto const& txHash : *m_transactionsHash)
    {
        m_pbRawBlock->set_transactionshash(index++, txHash.data(), h256::size);
    }
}
void PBBlock::encodeReceiptsHash() const
{
    auto receiptsHashNum = m_receiptsHash->size();
    if (receiptsHashNum == 0)
    {
        return;
    }
    // hit the cache
    if (m_pbRawBlock->receiptshash_size() > 0)
    {
        return;
    }
    // extend receiptsshash
    for (size_t i = 0; i < receiptsHashNum; i++)
    {
        m_pbRawBlock->add_receiptshash();
    }
    int index = 0;
    for (auto const& receiptHash : *m_receiptsHash)
    {
        m_pbRawBlock->set_receiptshash(index++, receiptHash.data(), h256::size);
    }
}

// getNonces of the current block
NonceListPtr PBBlock::nonces()
{
    auto nonceList = std::make_shared<NonceList>();
    if (m_transactions->size() == 0)
    {
        return nonceList;
    }
    for (auto transaction : *m_transactions)
    {
        nonceList->push_back(transaction->nonce());
    }
    return nonceList;
}

Transaction::ConstPtr PBBlock::transaction(size_t _index)
{
    if (m_transactions->size() < _index)
    {
        return nullptr;
    }
    return (*m_transactions)[_index];
}

h256 const& PBBlock::transactionHash(size_t _index)
{
    return (*m_transactionsHash)[_index];
}

TransactionReceipt::ConstPtr PBBlock::receipt(size_t _index)
{
    if (m_receipts->size() < _index)
    {
        return nullptr;
    }
    return (*m_receipts)[_index];
}

h256 const& PBBlock::receiptHash(size_t _index)
{
    return (*m_receiptsHash)[_index];
}