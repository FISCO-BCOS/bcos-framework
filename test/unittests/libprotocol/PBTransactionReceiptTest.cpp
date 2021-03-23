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
 * @brief test for Transaction
 * @file PBTransactionTest.h
 * @author: yujiechen
 * @date: 2021-03-16
 */
#include <bcos-crypto/hash/Keccak256.h>
#include <bcos-crypto/hash/SM3.h>
#include <bcos-framework/libprotocol/protobuf/PBTransactionReceiptFactory.h>
#include <bcos-framework/libutilities/Common.h>
#include <bcos-test/libutils/TestPromptFixture.h>
#include <boost/test/unit_test.hpp>

using namespace bcos;
using namespace bcos::protocol;
using namespace bcos::crypto;

namespace bcos
{
namespace test
{
BOOST_FIXTURE_TEST_SUITE(PBTransationReceiptTest, TestPromptFixture)
LogEntriesPtr fakeLogEntries(Hash::Ptr _hashImpl, size_t _size)
{
    LogEntriesPtr logEntries = std::make_shared<LogEntries>();
    for (size_t i = 0; i < _size; i++)
    {
        auto topic = _hashImpl->hash(std::to_string(i));
        h256s topics;
        topics.push_back(topic);
        auto address = right160(topic);
        bytes output = topic.asBytes();
        auto logEntry = std::make_shared<LogEntry>(address, topics, output);
        logEntries->push_back(logEntry);
    }
    return logEntries;
}

void testPBTransactionReceipt(CryptoSuite::Ptr _cryptoSuite)
{
    auto hashImpl = _cryptoSuite->hashImpl();
    int32_t version = 1;
    auto stateRoot = hashImpl->hash((std::string) "stateRoot");
    u256 gasUsed = 12343242342;
    auto contractAddress = toAddress("5fe3c4c3e2079879a0dba1937aca95ac16e68f0f");
    auto logEntries = fakeLogEntries(hashImpl, 2);
    TransactionStatus status = TransactionStatus::BadJumpDestination;
    bytes output = contractAddress.asBytes();
    for (int i = 0; i < 10; i++)
    {
        output += contractAddress.asBytes();
    }
    auto factory = std::make_shared<PBTransactionReceiptFactory>(_cryptoSuite);
    auto receipt = factory->createReceipt(
        version, stateRoot, gasUsed, contractAddress, logEntries, (int32_t)status, output);
    // encode
    std::shared_ptr<bytes> encodedData = std::make_shared<bytes>();
    auto start = utcTime();
    for (size_t i = 0; i < 200; i++)
    {
        receipt->encode(*encodedData);
    }
    std::cout << "##### ScaleReceipt encodeT: " << (utcTime() - start)
              << ", encodedData size:" << encodedData->size() << std::endl;

    // decode
    std::shared_ptr<TransactionReceipt> decodedReceipt;
    start = utcTime();
    for (size_t i = 0; i < 20000; i++)
    {
        decodedReceipt = factory->createReceipt(*encodedData);
    }
    std::cout << "##### ScaleReceipt decodeT: " << (utcTime() - start) << std::endl;

    // check the decodedReceipt
    BOOST_CHECK(decodedReceipt->version() == receipt->version());
    BOOST_CHECK(decodedReceipt->stateRoot() == receipt->stateRoot());
    BOOST_CHECK(decodedReceipt->gasUsed() == receipt->gasUsed());
    BOOST_CHECK(decodedReceipt->contractAddress() == receipt->contractAddress());
    BOOST_CHECK(decodedReceipt->status() == receipt->status());
    BOOST_CHECK(decodedReceipt->output().toBytes() == receipt->output().toBytes());
    BOOST_CHECK(decodedReceipt->hash() == receipt->hash());
    BOOST_CHECK(decodedReceipt->bloom() == receipt->bloom());
    // check LogEntries
    BOOST_CHECK(decodedReceipt->logEntries()->size() == 2);
    BOOST_CHECK(decodedReceipt->logEntries()->size() == receipt->logEntries()->size());
    auto logEntry = (*(decodedReceipt->logEntries()))[1];
    auto expectedTopic = hashImpl->hash(std::to_string(1));
    BOOST_CHECK(logEntry->topics()[0] == expectedTopic);
    BOOST_CHECK(logEntry->address() == right160(expectedTopic));
    BOOST_CHECK(logEntry->data() == expectedTopic.asBytes());

    std::cout << "#### encodedData: " << *toHexString(*encodedData) << std::endl;
    std::cout << "### stateRoot:" << decodedReceipt->stateRoot().hex() << std::endl;
    std::cout << "### gasUsed:" << decodedReceipt->gasUsed() << std::endl;
    std::cout << "### contractAddress:" << decodedReceipt->contractAddress().hex() << std::endl;
    std::cout << "### status:" << decodedReceipt->status() << std::endl;
    std::cout << "### output:" << *toHexString(decodedReceipt->output()) << std::endl;
    std::cout << "### hash:" << decodedReceipt->hash().hex() << std::endl;
    std::cout << "### bloom:" << decodedReceipt->bloom().hex() << std::endl;
    std::cout << "### topic:" << (logEntry->topics()[0]).hex() << std::endl;
    std::cout << "### log address:" << logEntry->address().hex() << std::endl;
    std::cout << "### log data:" << *toHexString(logEntry->data()) << std::endl;
}
BOOST_AUTO_TEST_CASE(testNormalPBransactionReceipt)
{
    auto hashImpl = std::make_shared<Keccak256>();
    auto cryptoSuite = std::make_shared<CryptoSuite>(hashImpl, nullptr, nullptr);
    testPBTransactionReceipt(cryptoSuite);
}
BOOST_AUTO_TEST_CASE(testSMPBTransactionReceipt)
{
    auto hashImpl = std::make_shared<SM3>();
    auto cryptoSuite = std::make_shared<CryptoSuite>(hashImpl, nullptr, nullptr);
    testPBTransactionReceipt(cryptoSuite);
}

BOOST_AUTO_TEST_CASE(testNormalPBTransactionRecept)
{
    auto hashImpl = std::make_shared<Keccak256>();
    auto cryptoSuite = std::make_shared<CryptoSuite>(hashImpl, nullptr, nullptr);
    auto receiptData = fromHexString(
        "08d0f181e10312d1050b00000071035fe3c4c3e2079879a0dba1937aca95ac16e68f0f5fe3c4c3e2079879a0db"
        "a1937aca95ac16e68f0f5fe3c4c3e2079879a0dba1937aca95ac16e68f0f5fe3c4c3e2079879a0dba1937aca95"
        "ac16e68f0f5fe3c4c3e2079879a0dba1937aca95ac16e68f0f5fe3c4c3e2079879a0dba1937aca95ac16e68f0f"
        "5fe3c4c3e2079879a0dba1937aca95ac16e68f0f5fe3c4c3e2079879a0dba1937aca95ac16e68f0f5fe3c4c3e2"
        "079879a0dba1937aca95ac16e68f0f5fe3c4c3e2079879a0dba1937aca95ac16e68f0f5fe3c4c3e2079879a0db"
        "a1937aca95ac16e68f0f505fe3c4c3e2079879a0dba1937aca95ac16e68f0f808ea407c24ec9663ee0e7a5f2d0"
        "e9afffec28087c25f296e68585bb292ed6c5e50766eeb6df020104000000000000000000000000000000000000"
        "000000000000100000000004000000000000000000000000000000000000000000000000000000000004100000"
        "000000000000000000000000000000000000000000080000000000000000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
        "000000000000000000800000000002000000000000000000000000000000000200000000000000000000080000"
        "000000000000000000000000000000000000000000000000000200000000080000000000000000000000000000"
        "0000000000000000000001000008502863c51de9fcb96542a07186fe3aeda6bb8a116d0480044852b2a670ade5"
        "407e78fb2863c51de9fcb96542a07186fe3aeda6bb8a116d80044852b2a670ade5407e78fb2863c51de9fcb965"
        "42a07186fe3aeda6bb8a116d5082df0950f5a951637e0307cdcb4c672f298b8bc60480c89efdaa54c0f20c7adf"
        "612882df0950f5a951637e0307cdcb4c672f298b8bc680c89efdaa54c0f20c7adf612882df0950f5a951637e03"
        "07cdcb4c672f298b8bc6");
    auto receipt = std::make_shared<PBTransactionReceipt>(cryptoSuite, *receiptData);
    BOOST_CHECK(receipt->stateRoot().hex() ==
                "8ea407c24ec9663ee0e7a5f2d0e9afffec28087c25f296e68585bb292ed6c5e5");
    BOOST_CHECK(receipt->gasUsed() == 12343242342);
    BOOST_CHECK(
        *toHexString(receipt->output()) ==
        "5fe3c4c3e2079879a0dba1937aca95ac16e68f0f5fe3c4c3e2079879a0dba1937aca95ac16e68f0f5fe3c4c3e2"
        "079879a0dba1937aca95ac16e68f0f5fe3c4c3e2079879a0dba1937aca95ac16e68f0f5fe3c4c3e2079879a0db"
        "a1937aca95ac16e68f0f5fe3c4c3e2079879a0dba1937aca95ac16e68f0f5fe3c4c3e2079879a0dba1937aca95"
        "ac16e68f0f5fe3c4c3e2079879a0dba1937aca95ac16e68f0f5fe3c4c3e2079879a0dba1937aca95ac16e68f0f"
        "5fe3c4c3e2079879a0dba1937aca95ac16e68f0f5fe3c4c3e2079879a0dba1937aca95ac16e68f0f");
    BOOST_CHECK(receipt->contractAddress().hex() == "5fe3c4c3e2079879a0dba1937aca95ac16e68f0f");
    BOOST_CHECK(receipt->status() == (int32_t)TransactionStatus::BadJumpDestination);
    BOOST_CHECK(receipt->hash().hex() ==
                "55237f2daa7721c03d0f014961ef3e85b45774ffc44a363d1c695e044d744934");
    BOOST_CHECK(
        receipt->bloom().hex() ==
        "000000000000000000000000000000000000000000000000100000000004000000000000000000000000000000"
        "000000000000000000000000000004100000000000000000000000000000000000000000000000080000000000"
        "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000800000000002000000000000000000000000"
        "000000000200000000000000000000080000000000000000000000000000000000000000000000000000000200"
        "00000008000000000000000000000000000000000000000000000000010000");
    auto logEntry = (*(receipt->logEntries()))[1];
    BOOST_CHECK((logEntry->topics()[0]).hex() ==
                "c89efdaa54c0f20c7adf612882df0950f5a951637e0307cdcb4c672f298b8bc6");
    BOOST_CHECK(logEntry->address().hex() == "82df0950f5a951637e0307cdcb4c672f298b8bc6");
    BOOST_CHECK(*toHexString(logEntry->data()) ==
                "c89efdaa54c0f20c7adf612882df0950f5a951637e0307cdcb4c672f298b8bc6");

    // check exception case
    (*receiptData)[0] += 1;
    BOOST_CHECK_THROW(
        std::make_shared<PBTransactionReceipt>(cryptoSuite, *receiptData), ReceiptDecodeException);
}

BOOST_AUTO_TEST_CASE(testSMPBTransactionRecept)
{
    auto hashImpl = std::make_shared<SM3>();
    auto cryptoSuite = std::make_shared<CryptoSuite>(hashImpl, nullptr, nullptr);
    auto receiptData = fromHexString(
        "088f9ffc9afeffffffff0112d1050b00000071035fe3c4c3e2079879a0dba1937aca95ac16e68f0f5fe3c4c3e2"
        "079879a0dba1937aca95ac16e68f0f5fe3c4c3e2079879a0dba1937aca95ac16e68f0f5fe3c4c3e2079879a0db"
        "a1937aca95ac16e68f0f5fe3c4c3e2079879a0dba1937aca95ac16e68f0f5fe3c4c3e2079879a0dba1937aca95"
        "ac16e68f0f5fe3c4c3e2079879a0dba1937aca95ac16e68f0f5fe3c4c3e2079879a0dba1937aca95ac16e68f0f"
        "5fe3c4c3e2079879a0dba1937aca95ac16e68f0f5fe3c4c3e2079879a0dba1937aca95ac16e68f0f5fe3c4c3e2"
        "079879a0dba1937aca95ac16e68f0f505fe3c4c3e2079879a0dba1937aca95ac16e68f0f800f000c0d3cf84963"
        "2ed9aa2925251c0a13ee4f09f991b30bf396c15bf7ae80970766eeb6df02010400000010000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000040100001000000000000000000000000000000000"
        "000000000000000000000000000000000000000040000000000000000000000000000000000000000000000000"
        "000000001000000000000000000000000000000000000000000010000000000000000000000000000000001000"
        "000000000004000000000000000000002000000000000000000000000000000000000000000000000000000000"
        "000000000000000000000000040020000000085072e58843de1eed164ed526c3b56b22c2b47324a0048006d47b"
        "6f2f121e85160d1d8072e58843de1eed164ed526c3b56b22c2b47324a08006d47b6f2f121e85160d1d8072e588"
        "43de1eed164ed526c3b56b22c2b47324a050d7d75330538a6882f5dfdc3b64115c647f3328c40480cbdddb8e84"
        "21b23498480570d7d75330538a6882f5dfdc3b64115c647f3328c480cbdddb8e8421b23498480570d7d7533053"
        "8a6882f5dfdc3b64115c647f3328c4");
    auto receipt = std::make_shared<PBTransactionReceipt>(cryptoSuite, *receiptData);
    BOOST_CHECK(receipt->stateRoot().hex() ==
                "0f000c0d3cf849632ed9aa2925251c0a13ee4f09f991b30bf396c15bf7ae8097");
    BOOST_CHECK(receipt->gasUsed() == 12343242342);
    BOOST_CHECK(
        *toHexString(receipt->output()) ==
        "5fe3c4c3e2079879a0dba1937aca95ac16e68f0f5fe3c4c3e2079879a0dba1937aca95ac16e68f0f5fe3c4c3e2"
        "079879a0dba1937aca95ac16e68f0f5fe3c4c3e2079879a0dba1937aca95ac16e68f0f5fe3c4c3e2079879a0db"
        "a1937aca95ac16e68f0f5fe3c4c3e2079879a0dba1937aca95ac16e68f0f5fe3c4c3e2079879a0dba1937aca95"
        "ac16e68f0f5fe3c4c3e2079879a0dba1937aca95ac16e68f0f5fe3c4c3e2079879a0dba1937aca95ac16e68f0f"
        "5fe3c4c3e2079879a0dba1937aca95ac16e68f0f5fe3c4c3e2079879a0dba1937aca95ac16e68f0f");
    BOOST_CHECK(receipt->contractAddress().hex() == "5fe3c4c3e2079879a0dba1937aca95ac16e68f0f");
    BOOST_CHECK(receipt->status() == (int32_t)TransactionStatus::BadJumpDestination);
    BOOST_CHECK(receipt->hash().hex() ==
                "32445e2828a4e081047cd70a3e9c2527f596d0ff2434d71fc48b131bef007567");
    BOOST_CHECK(
        receipt->bloom().hex() ==
        "000000100000000000000000000000000000000000000000000000000000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000000000000000401000010000000"
        "000000000000000000000000000000000000000000000000000000000000000000400000000000000000000000"
        "000000000000000000000000000000000010000000000000000000000000000000000000000000100000000000"
        "000000000000000000000010000000000000040000000000000000000020000000000000000000000000000000"
        "00000000000000000000000000000000000000000000000000040020000000");
    auto logEntry = (*(receipt->logEntries()))[1];
    BOOST_CHECK((logEntry->topics()[0]).hex() ==
                "cbdddb8e8421b23498480570d7d75330538a6882f5dfdc3b64115c647f3328c4");
    BOOST_CHECK(logEntry->address().hex() == "d7d75330538a6882f5dfdc3b64115c647f3328c4");
    BOOST_CHECK(*toHexString(logEntry->data()) ==
                "cbdddb8e8421b23498480570d7d75330538a6882f5dfdc3b64115c647f3328c4");
    // check exception case
    (*receiptData)[0] += 1;
    BOOST_CHECK_THROW(
        std::make_shared<PBTransactionReceipt>(cryptoSuite, *receiptData), ReceiptDecodeException);
}
BOOST_AUTO_TEST_SUITE_END()
}  // namespace test
}  // namespace bcos