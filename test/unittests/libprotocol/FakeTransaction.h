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
 * @file FakeTransaction.h
 * @author: yujiechen
 * @date: 2021-03-16
 */
#pragma once
#include "libprotocol/Exceptions.h"
#include "libprotocol/protobuf/PBTransactionFactory.h"
#include "libutilities/Common.h"
#include <bcos-test/libutils/HashImpl.h>
#include <bcos-test/libutils/SignatureImpl.h>
#include <boost/test/unit_test.hpp>

using namespace bcos;
using namespace bcos::crypto;
using namespace bcos::protocol;

namespace bcos
{
namespace test
{
inline PBTransaction::Ptr fakeTransaction(CryptoSuite::Ptr _cryptoSuite,
    KeyPairInterface::Ptr _keyPair, bytes const& _to, bytes const& _input, u256 const& _nonce,
    int64_t const& _blockLimit, std::string const& _chainId, std::string const& _groupId)
{
    auto pbTransaction = std::make_shared<PBTransaction>(
        _cryptoSuite, 1, _to, _input, _nonce, _blockLimit, _chainId, _groupId, utcTime());
    // set signature
    auto signData = _cryptoSuite->signatureImpl()->sign(_keyPair, pbTransaction->hash(), true);
    pbTransaction->updateSignature(bytesConstRef(signData->data(), signData->size()),
        _keyPair->address(_cryptoSuite->hashImpl()).asBytes());
    return pbTransaction;
}

inline void checkTransction(Transaction::ConstPtr pbTransaction, Transaction::ConstPtr decodedTransaction)
{
    // check the fields
    BOOST_CHECK(decodedTransaction->hash() == pbTransaction->hash());
    BOOST_CHECK(decodedTransaction->sender().toBytes() == pbTransaction->sender().toBytes());
    BOOST_CHECK(decodedTransaction->type() == pbTransaction->type());
    BOOST_CHECK(decodedTransaction->to().toBytes() == pbTransaction->to().toBytes());
    // check the transaction hash fields
    BOOST_CHECK(decodedTransaction->input().toBytes() == pbTransaction->input().toBytes());
    BOOST_CHECK(decodedTransaction->nonce() == pbTransaction->nonce());
    BOOST_CHECK(decodedTransaction->blockLimit() == pbTransaction->blockLimit());
    BOOST_CHECK(decodedTransaction->chainId() == pbTransaction->chainId());
    BOOST_CHECK(decodedTransaction->groupId() == pbTransaction->groupId());
}

inline Transaction::Ptr testTransaction(CryptoSuite::Ptr _cryptoSuite,
    KeyPairInterface::Ptr _keyPair, bytes const& _to, bytes const& _input, u256 const& _nonce,
    int64_t const& _blockLimit, std::string const& _chainId, std::string const& _groupId)
{
    auto factory = std::make_shared<PBTransactionFactory>(_cryptoSuite);
    auto pbTransaction = fakeTransaction(
        _cryptoSuite, _keyPair, _to, _input, _nonce, _blockLimit, _chainId, _groupId);
    if (_to == bytes())
    {
        BOOST_CHECK(pbTransaction->type() == TransactionType::ContractCreation);
    }
    else
    {
        BOOST_CHECK(pbTransaction->type() == TransactionType::MessageCall);
    }

    BOOST_CHECK(pbTransaction->sender().toBytes() == _keyPair->address(_cryptoSuite->hashImpl()).asBytes());
    // encode
    std::shared_ptr<bytes> encodedData = std::make_shared<bytes>();
    pbTransaction->encode(*encodedData);
    std::cout << "#### encodedData is:" << *toHexString(*encodedData) << std::endl;
    std::cout << "### hash:" << pbTransaction->hash().hex() << std::endl;
    std::cout << "### sender:" << *toHexString(pbTransaction->sender()) << std::endl;
    std::cout << "### type:" << pbTransaction->type() << std::endl;
    std::cout << "### to:" << *toHexString(pbTransaction->to()) << std::endl;
    // decode
    auto decodedTransaction = factory->createTransaction(*encodedData, true);
    checkTransction(pbTransaction, decodedTransaction);
    return decodedTransaction;
}

inline Transaction::Ptr fakeTransaction(CryptoSuite::Ptr _cryptoSuite)
{
    auto keyPair = _cryptoSuite->signatureImpl()->generateKeyPair();
    auto to = keyPair->address(_cryptoSuite->hashImpl()).asBytes();
    std::string inputStr = "testTransaction";
    bytes input = asBytes(inputStr);
    u256 nonce = 120012323;
    int64_t blockLimit = 1000023;
    std::string chainId = "chainId";
    std::string groupId = "groupId";
    return testTransaction(_cryptoSuite, keyPair, to, input, nonce, blockLimit, chainId, groupId);
}
}  // namespace test
}  // namespace bcos