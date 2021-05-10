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
#include "FakeBlock.h"
#include "libprotocol/TransactionSubmitResultFactoryImpl.h"
#include "libprotocol/TransactionSubmitResultImpl.h"
#include <bcos-test/libutils/TestPromptFixture.h>
#include <boost/test/unit_test.hpp>
#include <memory>
using namespace bcos;
using namespace bcos::protocol;
using namespace bcos::crypto;

namespace bcos
{
namespace test
{
BOOST_FIXTURE_TEST_SUITE(PBBlockTest, TestPromptFixture)
void testBlock(CryptoSuite::Ptr cryptoSuite, BlockFactory::Ptr blockFactory)
{
    auto block1 = std::dynamic_pointer_cast<PBBlock>(
        fakeAndCheckBlock(cryptoSuite, blockFactory, true, 10, 3, 7, 6));
    // without blockHeader
    auto block2 = std::dynamic_pointer_cast<PBBlock>(
        fakeAndCheckBlock(cryptoSuite, blockFactory, false, 10, 4, 8, 6));
    // update transactions
    TransactionsPtr txs1 = std::make_shared<Transactions>();
    for (int i = 0; i < 5; i++)
    {
        txs1->push_back(fakeTransaction(cryptoSuite));
    }
    block2->setTransactions(txs1);
    BOOST_CHECK(block2->transactions()->size() == 5);
    // encode
    auto encodedData = std::make_shared<bytes>();
    block2->encode(*encodedData);
    // decode and check
    auto decodedBlock = std::dynamic_pointer_cast<PBBlock>(blockFactory->createBlock(*encodedData));
    checkBlock(cryptoSuite, block2, decodedBlock);

    // update transactions to empty
    auto txs = std::make_shared<Transactions>();
    auto receipts = std::make_shared<Receipts>();
    for (int i = 0; i < 2; i++)
    {
        receipts->push_back(testPBTransactionReceipt(cryptoSuite));
    }
    block2->setTransactions(txs);
    block2->setReceipts(receipts);
    block2->setBlockHeader(testPBBlockHeader(cryptoSuite));
    BOOST_CHECK(block2->transactions()->size() == 0);
    BOOST_CHECK(block2->receipts()->size() == 2);
    block2->encode(*encodedData);
    // decode and check the block
    decodedBlock = std::dynamic_pointer_cast<PBBlock>(blockFactory->createBlock(*encodedData));
    checkBlock(cryptoSuite, block2, decodedBlock);
    BOOST_CHECK(decodedBlock->transactions()->size() == 0);
    BOOST_CHECK(decodedBlock->receipts()->size() == 2);
    decodedBlock->setTransactions(txs1);
    BOOST_CHECK(decodedBlock->transactions()->size() == 5);
    // test TransactionSubmitResult
    auto tx = (*txs1)[0];
    auto receipt = (*receipts)[0];
    TransactionSubmitResult::Ptr onChainResult =
        std::make_shared<TransactionSubmitResultImpl>(receipt, tx, 0, decodedBlock->blockHeader());
    BOOST_CHECK(onChainResult->transactionIndex() == 0);
    BOOST_CHECK(onChainResult->from().toBytes() == tx->sender().toBytes());
    BOOST_CHECK(onChainResult->to().toBytes() == tx->to().toBytes());
    BOOST_CHECK(onChainResult->txHash() == tx->hash());
    BOOST_CHECK(onChainResult->blockHash() == decodedBlock->blockHeader()->hash());
    BOOST_CHECK(onChainResult->blockNumber() == decodedBlock->blockHeader()->number());
    BOOST_CHECK(cryptoSuite->hash("123") == blockFactory->cryptoSuite()->hash("123"));

    auto factory = std::make_shared<TransactionSubmitResultFactoryImpl>();
    auto result = factory->createTxSubmitResult(receipt, tx, 0, decodedBlock->blockHeader());
    BOOST_CHECK(result->transactionIndex() == 0);
    BOOST_CHECK(result->from().toBytes() == tx->sender().toBytes());
    BOOST_CHECK(result->to().toBytes() == tx->to().toBytes());
    BOOST_CHECK(result->txHash() == tx->hash());
    BOOST_CHECK(result->blockHash() == decodedBlock->blockHeader()->hash());
    BOOST_CHECK(result->blockNumber() == decodedBlock->blockHeader()->number());
}
BOOST_AUTO_TEST_CASE(testNormalBlock)
{
    auto cryptoSuite = createNormalCryptoSuite();
    auto blockFactory = createBlockFactory(cryptoSuite);
    testBlock(cryptoSuite, blockFactory);
}

BOOST_AUTO_TEST_CASE(testSMBlock)
{
    auto cryptoSuite = createNormalCryptoSuite();
    auto blockFactory = createBlockFactory(cryptoSuite);
    testBlock(cryptoSuite, blockFactory);
}
BOOST_AUTO_TEST_SUITE_END()
}  // namespace test
}  // namespace bcos