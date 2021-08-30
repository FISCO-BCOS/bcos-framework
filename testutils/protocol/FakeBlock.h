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
 * @brief test for PBBlock
 * @file PBBlockTest.cpp
 * @author: yujiechen
 * @date: 2021-03-23
 */
#pragma once
#include "libprotocol/protobuf/PBBlock.h"
#include "libprotocol/protobuf/PBBlockFactory.h"
#include "testutils/crypto/HashImpl.h"
#include "testutils/protocol/FakeBlockHeader.h"
#include "testutils/protocol/FakeTransaction.h"
#include "testutils/protocol/FakeTransactionReceipt.h"
#include <boost/test/unit_test.hpp>
using namespace bcos;
using namespace bcos::protocol;
using namespace bcos::crypto;

namespace bcos
{
namespace test
{
inline CryptoSuite::Ptr createNormalCryptoSuite()
{
    auto hashImpl = std::make_shared<Keccak256Hash>();
    auto signImpl = std::make_shared<Secp256k1SignatureImpl>();
    return std::make_shared<CryptoSuite>(hashImpl, signImpl, nullptr);
}

inline CryptoSuite::Ptr createSMCryptoSuite()
{
    auto hashImpl = std::make_shared<Sm3Hash>();
    auto signImpl = std::make_shared<SM2SignatureImpl>();
    return std::make_shared<CryptoSuite>(hashImpl, signImpl, nullptr);
}

inline BlockFactory::Ptr createBlockFactory(CryptoSuite::Ptr _cryptoSuite)
{
    auto blockHeaderFactory = std::make_shared<PBBlockHeaderFactory>(_cryptoSuite);
    auto transactionFactory = std::make_shared<PBTransactionFactory>(_cryptoSuite);
    auto receiptFactory = std::make_shared<PBTransactionReceiptFactory>(_cryptoSuite);
    return std::make_shared<PBBlockFactory>(blockHeaderFactory, transactionFactory, receiptFactory);
}

inline void checkBlock(CryptoSuite::Ptr _cryptoSuite, Block::Ptr block, Block::Ptr decodedBlock)
{
    BOOST_CHECK(block->blockType() == decodedBlock->blockType());
    // check BlockHeader
    if (block->blockHeader())
    {
        checkBlockHeader(block->blockHeader(), decodedBlock->blockHeader());
    }
    // check transactions
    BOOST_CHECK_EQUAL(decodedBlock->transactionsSize(), block->transactionsSize());
    for (size_t i = 0; i < block->transactionsSize(); ++i)
    {
        checkTransaction(block->transaction(i), decodedBlock->transaction(i));
    }
    /*
    for (auto transaction : *(block->transactions()))
    {
        checkTransaction(transaction, (*(decodedBlock->transactions()))[index++]);
    }
    */
    // check receipts
    BOOST_CHECK_EQUAL(decodedBlock->receiptsSize(), block->receiptsSize());
    for (size_t i = 0; i < block->receiptsSize(); ++i)
    {
        checkReceipts(_cryptoSuite->hashImpl(), block->receipt(i), decodedBlock->receipt(i));
    }
    /*
    for (auto receipt : *(block->receipts()))
    {
        checkReceipts(_cryptoSuite->hashImpl(), receipt, (*(decodedBlock->receipts()))[index++]);
    }
    */
    for (size_t i = 0; i < decodedBlock->transactionsHashSize(); i++)
    {
        BOOST_CHECK(decodedBlock->transactionHash(i) == block->transactionHash(i));
        BOOST_CHECK(
            decodedBlock->transactionMetaData(i)->hash() == block->transactionMetaData(i)->hash());
        BOOST_CHECK(
            decodedBlock->transactionMetaData(i)->to() == block->transactionMetaData(i)->to());
    }
    // check receiptsRoot
    h256 originHash = h256();
    if (block->blockHeader())
    {
        originHash = block->blockHeader()->hash();
    }
    BOOST_CHECK(block->calculateReceiptRoot(true) == decodedBlock->calculateReceiptRoot(true));
    if (block->blockHeader())
    {
        BOOST_CHECK(block->blockHeader()->receiptsRoot() == block->calculateReceiptRoot(false));
        BOOST_CHECK(decodedBlock->blockHeader()->receiptsRoot() ==
                    decodedBlock->calculateReceiptRoot(false));
        BOOST_CHECK(decodedBlock->blockHeader()->hash() != originHash);
        originHash = block->blockHeader()->hash();
    }
    // check transactionsRoot
    BOOST_CHECK(
        block->calculateTransactionRoot(true) == decodedBlock->calculateTransactionRoot(true));
    if (block->blockHeader())
    {
        BOOST_CHECK(block->blockHeader()->txsRoot() == block->calculateTransactionRoot(false));
        BOOST_CHECK(
            decodedBlock->blockHeader()->txsRoot() == block->calculateTransactionRoot(false));
        BOOST_CHECK(decodedBlock->blockHeader()->hash() != originHash);
        originHash = decodedBlock->blockHeader()->hash();
    }
    // Check idempotence
    auto txsRoot = block->calculateTransactionRoot(true);
    auto receiptsRoot = block->calculateReceiptRoot(true);
    BOOST_CHECK(txsRoot == block->calculateTransactionRoot(false));
    BOOST_CHECK(receiptsRoot == block->calculateReceiptRoot(false));
    if (decodedBlock->blockHeader())
    {
        BOOST_CHECK(decodedBlock->blockHeader()->hash() == originHash);
    }
}

inline Block::Ptr fakeAndCheckBlock(CryptoSuite::Ptr _cryptoSuite, BlockFactory::Ptr _blockFactory,
    bool _withHeader, size_t _txsNum, size_t _txsHashNum)
{
    auto block = _blockFactory->createBlock();
    if (_withHeader)
    {
        auto blockHeader = testPBBlockHeader(_cryptoSuite);
        block->setBlockHeader(blockHeader);
    }

    block->setBlockType(CompleteBlock);
    // fake transactions
    for (size_t i = 0; i < _txsNum; i++)
    {
        auto tx = fakeTransaction(_cryptoSuite, utcTime() + i);
        block->appendTransaction(tx);
    }
    // fake receipts
    for (size_t i = 0; i < _txsNum; i++)
    {
        auto receipt = testPBTransactionReceipt(_cryptoSuite);
        block->setReceipt(i, receipt);
    }
    // fake txsHash
    for (size_t i = 0; i < _txsHashNum; i++)
    {
        auto content = "transaction: " + std::to_string(i);
        auto hash = _cryptoSuite->hashImpl()->hash(content);
        auto txMetaData = _blockFactory->createTransactionMetaData(hash, *toHexString(hash));
        block->appendTransactionMetaData(txMetaData);
    }

    // encode block
    auto encodedData = std::make_shared<bytes>();
    block->encode(*encodedData);
    // decode block
    auto decodedBlock = _blockFactory->createBlock(*encodedData, true, true);
    checkBlock(_cryptoSuite, block, decodedBlock);
    // check txsHash
    BOOST_CHECK(decodedBlock->transactionsHashSize() == _txsHashNum);
    BOOST_CHECK(decodedBlock->transactionsMetaDataSize() == _txsHashNum);
    for (size_t i = 0; i < _txsHashNum; i++)
    {
        auto content = "transaction: " + std::to_string(i);
        auto hash = _cryptoSuite->hashImpl()->hash(content);
        BOOST_CHECK(decodedBlock->transactionHash(i) == hash);
        BOOST_CHECK(decodedBlock->transactionMetaData(i)->hash() == hash);
        BOOST_CHECK(decodedBlock->transactionMetaData(i)->to() == *toHexString(hash));
    }

    // exception test
    (*encodedData)[0] += 1;
    BOOST_CHECK_THROW(
        _blockFactory->createBlock(*encodedData, true, true), PBObjectDecodeException);
    return block;
}


}  // namespace test
}  // namespace bcos
