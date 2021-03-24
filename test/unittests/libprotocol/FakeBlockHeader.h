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
 * @file FakeBlockHeader.h
 * @author: yujiechen
 * @date: 2021-03-16
 */
#pragma once
#include <bcos-crypto/hash/Keccak256.h>
#include <bcos-crypto/hash/SM3.h>
#include <bcos-crypto/signature/secp256k1/Secp256k1Crypto.h>
#include <bcos-crypto/signature/sm2/SM2Crypto.h>
#include <bcos-framework/libprotocol/Exceptions.h>
#include <bcos-framework/libprotocol/protobuf/PBBlockHeaderFactory.h>
#include <bcos-framework/libutilities/Common.h>
#include <boost/test/unit_test.hpp>
using namespace bcos;
using namespace bcos::protocol;
using namespace bcos::crypto;
namespace bcos
{
namespace test
{
inline void checkBlockHeader(BlockHeader::Ptr blockHeader, BlockHeader::Ptr decodedBlockHeader)
{
    BOOST_CHECK(decodedBlockHeader->version() == blockHeader->version());
    BOOST_CHECK(*(decodedBlockHeader->parentInfo()) == *(blockHeader->parentInfo()));
    BOOST_CHECK(decodedBlockHeader->txsRoot() == blockHeader->txsRoot());
    BOOST_CHECK(decodedBlockHeader->receiptRoot() == blockHeader->receiptRoot());
    BOOST_CHECK(decodedBlockHeader->stateRoot() == blockHeader->stateRoot());
    BOOST_CHECK(decodedBlockHeader->number() == blockHeader->number());
    BOOST_CHECK(decodedBlockHeader->gasUsed() == blockHeader->gasUsed());
    BOOST_CHECK(decodedBlockHeader->timestamp() == blockHeader->timestamp());
    BOOST_CHECK(decodedBlockHeader->sealer() == blockHeader->sealer());
    BOOST_CHECK(*(decodedBlockHeader->sealerList()) == *(blockHeader->sealerList()));
    BOOST_CHECK(decodedBlockHeader->extraData() == blockHeader->extraData());
    // check signature
    BOOST_CHECK(
        decodedBlockHeader->signatureList()->size() == blockHeader->signatureList()->size());
    size_t index = 0;
    for (auto signature : *(decodedBlockHeader->signatureList()))
    {
        BOOST_CHECK(signature.first == (*(blockHeader->signatureList()))[index].first);
        BOOST_CHECK(*(signature.second) == *(*(blockHeader->signatureList()))[index].second);
        std::cout << "#### signatureData:" << *toHexString(*(signature.second)) << std::endl;
        index++;
    }
    // std::cout << "### PBBlockHeaderTest: encodedData:" << *toHexString(*encodedData) <<
    // std::endl;
    std::cout << "### PBBlockHeaderTest: version:" << decodedBlockHeader->version() << std::endl;
    std::cout << "### PBBlockHeaderTest: txsRoot:" << decodedBlockHeader->txsRoot().hex()
              << std::endl;
    std::cout << "### PBBlockHeaderTest: receiptRoot:" << decodedBlockHeader->receiptRoot().hex()
              << std::endl;
    std::cout << "### PBBlockHeaderTest: stateRoot:" << decodedBlockHeader->stateRoot().hex()
              << std::endl;
    std::cout << "### PBBlockHeaderTest: number:" << decodedBlockHeader->number() << std::endl;
    std::cout << "### PBBlockHeaderTest: gasUsed:" << decodedBlockHeader->gasUsed() << std::endl;
    std::cout << "### PBBlockHeaderTest: timestamp:" << decodedBlockHeader->timestamp()
              << std::endl;
    std::cout << "### PBBlockHeaderTest: sealer:" << decodedBlockHeader->sealer() << std::endl;
    std::cout << "### PBBlockHeaderTest: sealer:" << *toHexString(decodedBlockHeader->extraData())
              << std::endl;
    std::cout << "#### hash:" << decodedBlockHeader->hash().hex() << std::endl;
    std::cout << "### parentHash:" << (*(blockHeader->parentInfo()))[0].second.hex() << std::endl;
    for (auto const& sealer : *(decodedBlockHeader->sealerList()))
    {
        std::cout << "#### sealerList:" << sealer.hex() << std::endl;
    }
}

inline BlockHeader::Ptr fakeAndTestBlockHeader(CryptoSuite::Ptr _cryptoSuite, int32_t _version,
    ParentInfoListPtr _parentInfo, h256 const& _txsRoot, h256 const& _receiptRoot,
    h256 const& _stateRoot, int64_t _number, u256 const& _gasUsed, int64_t _timestamp,
    int64_t _sealer, SealerListPtr _sealerList, bytes const& _extraData,
    SignatureListPtr _signatureList)
{
    BlockHeaderFactory::Ptr blockHeaderFactory =
        std::make_shared<PBBlockHeaderFactory>(_cryptoSuite);
    BlockHeader::Ptr blockHeader = blockHeaderFactory->createBlockHeader();
    blockHeader->setVersion(_version);
    blockHeader->setParentInfo(_parentInfo);
    blockHeader->setTxsRoot(_txsRoot);
    blockHeader->setReceiptRoot(_receiptRoot);
    blockHeader->setStateRoot(_stateRoot);
    blockHeader->setNumber(_number);
    blockHeader->setGasUsed(_gasUsed);
    blockHeader->setTimestamp(_timestamp);
    blockHeader->setSealer(_sealer);
    blockHeader->setSealerList(*_sealerList);
    blockHeader->setExtraData(_extraData);
    blockHeader->setSignatureList(_signatureList);

    // encode
    std::shared_ptr<bytes> encodedData = std::make_shared<bytes>();
    blockHeader->encode(*encodedData);

    // decode
    auto decodedBlockHeader = blockHeaderFactory->createBlockHeader(*encodedData);
    // check the data of decodedBlockHeader
    checkBlockHeader(blockHeader, decodedBlockHeader);
    // test encode exception
    (*encodedData)[0] += 1;
    BOOST_CHECK_THROW(
        std::make_shared<PBBlockHeader>(_cryptoSuite, *encodedData), BlockHeaderEncodeException);

    // update the hash data field
    blockHeader->setNumber(_number + 1);
    BOOST_CHECK(blockHeader->hash() != decodedBlockHeader->hash());
    BOOST_CHECK(blockHeader->number() == decodedBlockHeader->number() + 1);
    // recover the hash field
    blockHeader->setNumber(_number);
    BOOST_CHECK(blockHeader->hash() == decodedBlockHeader->hash());
    return blockHeader;
}

inline ParentInfoListPtr fakeParentInfo(Hash::Ptr _hashImpl, size_t _size)
{
    ParentInfoListPtr parentInfos = std::make_shared<ParentInfoList>();
    for (size_t i = 0; i < _size; i++)
    {
        parentInfos->emplace_back(std::make_pair(i, _hashImpl->hash(std::to_string(i))));
    }
    return parentInfos;
}

inline SealerListPtr fakeSealerList(
    std::vector<KeyPair::Ptr>& _keyPairVec, SignatureCrypto::Ptr _signImpl, size_t size)
{
    SealerListPtr sealerList = std::make_shared<SealerList>();
    for (size_t i = 0; i < size; i++)
    {
        auto keyPair = _signImpl->generateKeyPair();
        _keyPairVec.emplace_back(keyPair);
        sealerList->emplace_back(keyPair->publicKey());
    }
    return sealerList;
}

inline SignatureListPtr fakeSignatureList(
    SignatureCrypto::Ptr _signImpl, std::vector<KeyPair::Ptr>& _keyPairVec, h256 const& _hash)
{
    auto sealerIndex = 0;
    auto signatureList = std::make_shared<SignatureList>();
    for (auto keyPair : _keyPairVec)
    {
        auto signature = _signImpl->sign(*keyPair, _hash);
        signatureList->push_back(std::make_pair(sealerIndex++, signature));
    }
    return signatureList;
}

inline BlockHeader::Ptr testPBBlockHeader(CryptoSuite::Ptr _cryptoSuite)
{
    auto hashImpl = _cryptoSuite->hashImpl();
    auto signImpl = _cryptoSuite->signatureImpl();
    auto cryptoSuite = std::make_shared<CryptoSuite>(hashImpl, signImpl, nullptr);
    int version = 10;
    ParentInfoListPtr parentInfo = fakeParentInfo(hashImpl, 3);
    auto txsRoot = hashImpl->hash((std::string) "txsRoot");
    auto receiptRoot = hashImpl->hash((std::string) "receiptRoot");
    auto stateRoot = hashImpl->hash((std::string) "stateRoot");
    int64_t number = 12312312412;
    u256 gasUsed = 3453456346534;
    int64_t timestamp = 9234234234;
    int64_t sealer = 100;
    std::vector<KeyPair::Ptr> keyPairVec;
    auto sealerList = fakeSealerList(keyPairVec, signImpl, 4);
    bytes extraData = stateRoot.asBytes();
    auto signatureList = fakeSignatureList(signImpl, keyPairVec, receiptRoot);

    auto blockHeader =
        fakeAndTestBlockHeader(cryptoSuite, version, parentInfo, txsRoot, receiptRoot, stateRoot,
            number, gasUsed, timestamp, sealer, sealerList, extraData, signatureList);

    // test verifySignatureList
    signatureList = fakeSignatureList(signImpl, keyPairVec, blockHeader->hash());
    blockHeader->setSignatureList(signatureList);
    blockHeader->verifySignatureList();

    auto invalidSignatureList = fakeSignatureList(signImpl, keyPairVec, receiptRoot);
    blockHeader->setSignatureList(invalidSignatureList);
    BOOST_CHECK_THROW(blockHeader->verifySignatureList(), InvalidSignatureList);

    blockHeader->setSignatureList(signatureList);
    blockHeader->verifySignatureList();
    return blockHeader;
}
}  // namespace test
}  // namespace bcos
