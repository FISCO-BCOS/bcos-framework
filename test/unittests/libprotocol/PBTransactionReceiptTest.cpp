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
#include "FakeTransactionReceipt.h"
#include "libprotocol/Common.h"
#include <bcos-test/libutils/TestPromptFixture.h>

using namespace bcos;
using namespace bcos::protocol;
using namespace bcos::crypto;

namespace bcos
{
namespace test
{
BOOST_FIXTURE_TEST_SUITE(PBTransationReceiptTest, TestPromptFixture)
BOOST_AUTO_TEST_CASE(testNormalPBransactionReceipt)
{
    auto hashImpl = std::make_shared<Keccak256Hash>();
    auto cryptoSuite = std::make_shared<CryptoSuite>(hashImpl, nullptr, nullptr);
    testPBTransactionReceipt(cryptoSuite);
}
BOOST_AUTO_TEST_CASE(testSMPBTransactionReceipt)
{
    auto hashImpl = std::make_shared<Sm3Hash>();
    auto cryptoSuite = std::make_shared<CryptoSuite>(hashImpl, nullptr, nullptr);
    testPBTransactionReceipt(cryptoSuite);
}

BOOST_AUTO_TEST_CASE(testNormalPBTransactionRecept)
{
    auto hashImpl = std::make_shared<Keccak256Hash>();
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
    BOOST_CHECK(
        *toHexString(receipt->contractAddress()) == "5fe3c4c3e2079879a0dba1937aca95ac16e68f0f");
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
    BOOST_CHECK(*toHexString(logEntry->address()) == "82df0950f5a951637e0307cdcb4c672f298b8bc6");
    BOOST_CHECK(*toHexString(logEntry->data()) ==
                "c89efdaa54c0f20c7adf612882df0950f5a951637e0307cdcb4c672f298b8bc6");

    // check exception case
    (*receiptData)[0] += 1;
    BOOST_CHECK_THROW(
        std::make_shared<PBTransactionReceipt>(cryptoSuite, *receiptData), PBObjectDecodeException);
}

BOOST_AUTO_TEST_CASE(testSMPBTransactionRecept)
{
    auto hashImpl = std::make_shared<Sm3Hash>();
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
    BOOST_CHECK(
        *toHexString(receipt->contractAddress()) == "5fe3c4c3e2079879a0dba1937aca95ac16e68f0f");
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
    BOOST_CHECK(*toHexString(logEntry->address()) == "d7d75330538a6882f5dfdc3b64115c647f3328c4");
    BOOST_CHECK(*toHexString(logEntry->data()) ==
                "cbdddb8e8421b23498480570d7d75330538a6882f5dfdc3b64115c647f3328c4");
    // check exception case
    (*receiptData)[0] += 1;
    BOOST_CHECK_THROW(
        std::make_shared<PBTransactionReceipt>(cryptoSuite, *receiptData), PBObjectDecodeException);
}
BOOST_AUTO_TEST_SUITE_END()
}  // namespace test
}  // namespace bcos