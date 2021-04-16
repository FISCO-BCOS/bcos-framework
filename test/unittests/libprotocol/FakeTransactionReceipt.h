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
 * @file FakeTransactionReceipt.h
 * @author: yujiechen
 * @date: 2021-03-16
 */
#pragma once
#include "libprotocol/protobuf/PBTransactionReceiptFactory.h"
#include "libutilities/Common.h"
#include <bcos-test/libutils/HashImpl.h>
#include <boost/test/unit_test.hpp>

using namespace bcos;
using namespace bcos::protocol;
using namespace bcos::crypto;

namespace bcos
{
namespace test
{
inline LogEntriesPtr fakeLogEntries(Hash::Ptr _hashImpl, size_t _size)
{
    LogEntriesPtr logEntries = std::make_shared<LogEntries>();
    for (size_t i = 0; i < _size; i++)
    {
        auto topic = _hashImpl->hash(std::to_string(i));
        h256s topics;
        topics.push_back(topic);
        auto address = right160(topic).asBytes();
        bytes output = topic.asBytes();
        auto logEntry = std::make_shared<LogEntry>(address, topics, output);
        logEntries->push_back(logEntry);
    }
    return logEntries;
}

inline void checkReceipts(
    Hash::Ptr hashImpl, TransactionReceipt::Ptr receipt, TransactionReceipt::Ptr decodedReceipt)
{
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
    BOOST_CHECK(logEntry->address() == right160(expectedTopic).asBytes());
    BOOST_CHECK(logEntry->data() == expectedTopic.asBytes());
}
inline TransactionReceipt::Ptr testPBTransactionReceipt(CryptoSuite::Ptr _cryptoSuite)
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
    auto receipt = factory->createReceipt(version, stateRoot, gasUsed, contractAddress.asBytes(),
        logEntries, (int32_t)status, output);
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
    checkReceipts(hashImpl, receipt, decodedReceipt);
    return decodedReceipt;
}
}  // namespace test
}  // namespace bcos