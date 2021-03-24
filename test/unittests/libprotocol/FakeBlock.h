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
#include "FakeBlockHeader.h"
#include "FakeTransaction.h"
#include "FakeTransactionReceipt.h"
#include <bcos-framework/libprotocol/protobuf/PBBlock.h>
#include <bcos-framework/libprotocol/protobuf/PBBlockFactory.h>
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
    auto hashImpl = std::make_shared<Keccak256>();
    auto signImpl = std::make_shared<Secp256k1Crypto>();
    return std::make_shared<CryptoSuite>(hashImpl, signImpl, nullptr);
}

inline CryptoSuite::Ptr createSMCryptoSuite()
{
    auto hashImpl = std::make_shared<SM3>();
    auto signImpl = std::make_shared<SM2Crypto>();
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
    BOOST_CHECK(decodedBlock->transactions()->size() == block->transactions()->size());
    size_t index = 0;
    for (auto transaction : *(block->transactions()))
    {
        checkTransction(transaction, (*(decodedBlock->transactions()))[index++]);
    }
    // check receipts
    BOOST_CHECK(decodedBlock->receipts()->size() == block->receipts()->size());
    index = 0;
    for (auto receipt : *(block->receipts()))
    {
        checkReceipts(_cryptoSuite->hashImpl(), receipt, (*(decodedBlock->receipts()))[index++]);
    }
    // checkHash
    for (size_t i = 0; i < decodedBlock->receiptsHash()->size(); i++)
    {
        BOOST_CHECK((*(decodedBlock->receiptsHash()))[i] == (*(block->receiptsHash()))[i]);
    }
    for (size_t i = 0; i < decodedBlock->transactionsHash()->size(); i++)
    {
        BOOST_CHECK((*(decodedBlock->transactionsHash()))[i] == (*(block->transactionsHash()))[i]);
    }
}

inline Block::Ptr fakeAndCheckBlock(CryptoSuite::Ptr _cryptoSuite, BlockFactory::Ptr _blockFactory,
    bool _withHeader, size_t _txsNum, size_t _receiptsNum, size_t _txsHashNum,
    size_t _receiptsHashNum)
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
        auto tx = fakeTransaction(_cryptoSuite);
        block->appendTransaction(tx);
    }
    // fake receipts
    for (size_t i = 0; i < _receiptsNum; i++)
    {
        auto receipt = testPBTransactionReceipt(_cryptoSuite);
        block->appendReceipt(receipt);
    }
    // fake txsHash
    for (size_t i = 0; i < _txsHashNum; i++)
    {
        auto content = "transaction: " + std::to_string(i);
        auto hash = _cryptoSuite->hashImpl()->hash(content);
        block->appendTransactionHash(hash);
    }
    // fake receiptsHash
    for (size_t i = 0; i < _receiptsHashNum; i++)
    {
        auto content = "receipt: " + std::to_string(i);
        auto hash = _cryptoSuite->hashImpl()->hash(content);
        block->appendReceiptHash(hash);
    }
    // encode block
    auto encodedData = std::make_shared<bytes>();
    block->encode(*encodedData);
    // decode block
    auto decodedBlock = _blockFactory->createBlock(*encodedData, true, true);
    BOOST_CHECK(CompleteBlock == decodedBlock->blockType());
    checkBlock(_cryptoSuite, block, decodedBlock);
    // check txsHash
    BOOST_CHECK(decodedBlock->transactionsHash()->size() == _txsHashNum);
    for (size_t i = 0; i < _txsHashNum; i++)
    {
        auto content = "transaction: " + std::to_string(i);
        auto hash = _cryptoSuite->hashImpl()->hash(content);
        BOOST_CHECK((*(decodedBlock->transactionsHash()))[i] == hash);
    }
    // check receiptHash
    BOOST_CHECK(decodedBlock->receiptsHash()->size() == _receiptsHashNum);
    for (size_t i = 0; i < _receiptsHashNum; i++)
    {
        auto content = "receipt: " + std::to_string(i);
        auto hash = _cryptoSuite->hashImpl()->hash(content);
        BOOST_CHECK((*(decodedBlock->receiptsHash()))[i] == hash);
    }
    return block;
}


}  // namespace test
}  // namespace bcos