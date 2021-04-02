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
 * @brief test for BlockHeader
 * @file PBBlockHeaderTest.h
 * @author: yujiechen
 * @date: 2021-03-16
 */
#include "FakeBlockHeader.h"
#include <bcos-test/libutils/TestPromptFixture.h>

using namespace bcos;
using namespace bcos::protocol;
using namespace bcos::crypto;

namespace bcos
{
namespace test
{
BOOST_FIXTURE_TEST_SUITE(PBBlockHeaderTest, TestPromptFixture)
BOOST_AUTO_TEST_CASE(testNormalPBBlockHeader)
{
    auto hashImpl = std::make_shared<Keccak256Hash>();
    auto signImpl = std::make_shared<Secp256k1SignatureImpl>();
    auto cryptoSuite = std::make_shared<CryptoSuite>(hashImpl, signImpl, nullptr);
    testPBBlockHeader(cryptoSuite);
}

BOOST_AUTO_TEST_CASE(testSMPBBlockHeader)
{
    auto hashImpl = std::make_shared<Sm3Hash>();
    auto signImpl = std::make_shared<SM2SignatureImpl>();
    auto cryptoSuite = std::make_shared<CryptoSuite>(hashImpl, signImpl, nullptr);
    testPBBlockHeader(cryptoSuite);
}

BOOST_AUTO_TEST_CASE(testRawPBBlockHeader)
{
    auto hashImpl = std::make_shared<Keccak256Hash>();
    auto signImpl = std::make_shared<Secp256k1SignatureImpl>();
    auto cryptoSuite = std::make_shared<CryptoSuite>(hashImpl, signImpl, nullptr);
    auto data = fromHexString(
        "080a12a8040c000000000000000080044852b2a670ade5407e78fb2863c51de9fcb96542a07186fe3aeda6bb8a"
        "116d010000000000000080c89efdaa54c0f20c7adf612882df0950f5a951637e0307cdcb4c672f298b8bc60200"
        "00000000000080ad7c5bef027816a800da1736444fb58a807ef4c9603b7848673f7e3a68eb14a5808fc1d10c13"
        "0a57800c446a858cf3bb4699c25d17f6318484814f18a9e1227f1b80fbc6d2ca221fb45933acae4194e9c1f132"
        "bd6565e678e3a21d088b2545d11c24808ea407c24ec9663ee0e7a5f2d0e9afffec28087c25f296e68585bb292e"
        "d6c5e55cfadedd020000000ba6ed091224037a3b672602000000640000000000000010010103705470fe0dbf39"
        "0f16a00a08135bcbaa3886b250c553a472168e5edc5144a3ad217da7f43b956b249ea9cc1415841df4283fb07c"
        "e1a07021debfe0fbca038e010179a30acb90df081b43bdb18bca1c8bbb6421f62c86b615abe539c2d9cd6df17f"
        "ec46c64b8412cd1a65e3c2e219531e73da8253654878490e0bc9e5c52f2083510101e61d6d6d35ab384c49ac4f"
        "620aa25f78601b326b33a66132316cd2b99eacc44c19eabdfa2c25213b2959aa67b9180038d40d41007bd76b10"
        "f9b84e76174159310101d7af5bfde217dc000037944803e4383a019dbbb4c2ef4eed109c180ff6ed4a85a9f288"
        "290c82e72bfcd46eefb5583fe60206d9423583731450baad3bb559abe7808ea407c24ec9663ee0e7a5f2d0e9af"
        "ffec28087c25f296e68585bb292ed6c5e51a43124169c93bc8a1987698e8da61366512c918e7019ec52213689f"
        "869cf1605ef55c0d43032a88cac1e068447eed7856dc2b89979601c18c9e33cd7627453daef72e87001a450801"
        "1241a56d6b451842f5d8d040125f97fc0cad118ee9366b11110db0d10b77301a38377e45ed8b6c9d5d80ad3ab6"
        "2e6e13162eab349a6175b2f615e5e6f4f61367473c011a4508021241496d150934a3465fb590877ae9147c5308"
        "c5370a12ff8eaf0bc7d8cd2e39d2d753adac4c90ca5cf8b5b3736bf9aba97304fd8f43e3c5d173b6d87d1e0ac8"
        "9d8a011a4508031241c1694c5052cdddbfcc2219af7000ab5b0d4de0833ffefba044c0aadba8ee6e4827da216e"
        "fb969294be677db95a5e7d616534b3076ae22cfe4392a32b6337d84601");
    auto decodedBlockHeader = std::make_shared<PBBlockHeader>(cryptoSuite, *data);

    BOOST_CHECK(decodedBlockHeader->version() == 10);
    BOOST_CHECK(decodedBlockHeader->txsRoot().hex() ==
                "8fc1d10c130a57800c446a858cf3bb4699c25d17f6318484814f18a9e1227f1b");
    BOOST_CHECK(decodedBlockHeader->receiptRoot().hex() ==
                "fbc6d2ca221fb45933acae4194e9c1f132bd6565e678e3a21d088b2545d11c24");
    BOOST_CHECK(decodedBlockHeader->stateRoot().hex() ==
                "8ea407c24ec9663ee0e7a5f2d0e9afffec28087c25f296e68585bb292ed6c5e5");
    BOOST_CHECK(decodedBlockHeader->number() == 12312312412);
    BOOST_CHECK(decodedBlockHeader->gasUsed() == 3453456346534);
    BOOST_CHECK(decodedBlockHeader->timestamp() == 9234234234);
    BOOST_CHECK(decodedBlockHeader->sealer() == 100);
    BOOST_CHECK(*toHexString(decodedBlockHeader->extraData()) ==
                "8ea407c24ec9663ee0e7a5f2d0e9afffec28087c25f296e68585bb292ed6c5e5");
    // check signature
    BOOST_CHECK(decodedBlockHeader->signatureList()->size() == 4);
    BOOST_CHECK(decodedBlockHeader->hash().hex() ==
                "2a23582ab11d58319189cd727949c4248da866771e467e8cfd74f24958253c4d");
    BOOST_CHECK((*decodedBlockHeader->parentInfo())[0].second.hex() ==
                "044852b2a670ade5407e78fb2863c51de9fcb96542a07186fe3aeda6bb8a116d");
    BOOST_CHECK(*toHexString(*(*decodedBlockHeader->sealerList())[0]) ==
                "03705470fe0dbf390f16a00a08135bcbaa3886b250c553a472168e5edc5144a3ad217da7f43b956b24"
                "9ea9cc1415841df4283fb07ce1a07021debfe0fbca038e");
    BOOST_CHECK(*toHexString(*(*decodedBlockHeader->sealerList())[1]) ==
                "79a30acb90df081b43bdb18bca1c8bbb6421f62c86b615abe539c2d9cd6df17fec46c64b8412cd1a65"
                "e3c2e219531e73da8253654878490e0bc9e5c52f208351");
    BOOST_CHECK(*toHexString(*(*decodedBlockHeader->sealerList())[2]) ==
                "e61d6d6d35ab384c49ac4f620aa25f78601b326b33a66132316cd2b99eacc44c19eabdfa2c25213b29"
                "59aa67b9180038d40d41007bd76b10f9b84e7617415931");
    BOOST_CHECK(*toHexString(*(*decodedBlockHeader->sealerList())[3]) ==
                "d7af5bfde217dc000037944803e4383a019dbbb4c2ef4eed109c180ff6ed4a85a9f288290c82e72bfc"
                "d46eefb5583fe60206d9423583731450baad3bb559abe7");

    BOOST_CHECK(*toHexString(*(*decodedBlockHeader->signatureList())[0].second) ==
                "69c93bc8a1987698e8da61366512c918e7019ec52213689f869cf1605ef55c0d43032a88cac1e06844"
                "7eed7856dc2b89979601c18c9e33cd7627453daef72e8700");
    BOOST_CHECK(*toHexString(*(*decodedBlockHeader->signatureList())[1].second) ==
                "a56d6b451842f5d8d040125f97fc0cad118ee9366b11110db0d10b77301a38377e45ed8b6c9d5d80ad"
                "3ab62e6e13162eab349a6175b2f615e5e6f4f61367473c01");
    BOOST_CHECK(*toHexString(*(*decodedBlockHeader->signatureList())[2].second) ==
                "496d150934a3465fb590877ae9147c5308c5370a12ff8eaf0bc7d8cd2e39d2d753adac4c90ca5cf8b5"
                "b3736bf9aba97304fd8f43e3c5d173b6d87d1e0ac89d8a01");
    BOOST_CHECK(*toHexString(*(*decodedBlockHeader->signatureList())[3].second) ==
                "c1694c5052cdddbfcc2219af7000ab5b0d4de0833ffefba044c0aadba8ee6e4827da216efb969294be"
                "677db95a5e7d616534b3076ae22cfe4392a32b6337d84601");
}
BOOST_AUTO_TEST_CASE(testRawSMPBBlockHeader)
{
    auto hashImpl = std::make_shared<Sm3Hash>();
    auto signImpl = std::make_shared<SM2SignatureImpl>();
    auto cryptoSuite = std::make_shared<CryptoSuite>(hashImpl, signImpl, nullptr);

    auto data = fromHexString(
        "080a12a8040c00000000000000008006d47b6f2f121e85160d1d8072e58843de1eed164ed526c3b56b22c2b473"
        "24a0010000000000000080cbdddb8e8421b23498480570d7d75330538a6882f5dfdc3b64115c647f3328c40200"
        "00000000000080a0dc2d74b9b0e3c87e076003dbfe472a424cb3032463cb339e351460765a822e80bcd22fd79b"
        "459e46e3fbe2d1c63eb0421ef14479c5f187fa1230459e4052215d8036433bdcdf6144fa6abee2ffccf57897f6"
        "33a1f45e0f82e7796f25fed1b2f94b800f000c0d3cf849632ed9aa2925251c0a13ee4f09f991b30bf396c15bf7"
        "ae80975cfadedd020000000ba6ed091224037a3b6726020000006400000000000000100101676ebe6db5ab41f0"
        "3a69eda2b5d168dbef1a39d45c02bcda519ddc142831c745d48232b533fdab95d0a9c4a949bfbda1654efb5a26"
        "dca4028c5fdd26988cb22b0101d8f7cf8d26bbc32b78ce60da7a37c57a1764ff4632febb4dbc037ae2829375e2"
        "36443ca64b0339ad834f3eeda05172f7d580a6d3cbc82939699da95b91a927f70101c737cd87858df22fb12999"
        "b5cba3fb13cb03195bf79c816ad8fcf906b12ed24ffb0ef9c6dd591b83526a2d0c47bd40abb0df8104ea0765b9"
        "a70496d7d2316fcb010175dd6eb3679fce6944e808304eb79a5e0e37eea7a530d4f59f4ba7edfab79864fd9198"
        "8ce82ffc1c6be84712b81bd29325fb247af2f6698d9c180fc36a07344a800f000c0d3cf849632ed9aa2925251c"
        "0a13ee4f09f991b30bf396c15bf7ae80971a83011280016b36496b3501cb68faac7ed258a1c0fd0c9a97711548"
        "2720b9e322ee0a74a94ef4e4297863136eb7dd53581c8cb69bd4f134a3a2756ab3ddb38d30bf3529bbec676ebe"
        "6db5ab41f03a69eda2b5d168dbef1a39d45c02bcda519ddc142831c745d48232b533fdab95d0a9c4a949bfbda1"
        "654efb5a26dca4028c5fdd26988cb22b1a8501080112800195e6b6ba6516d1fe2b124fe673e33dd51dc39fce17"
        "d973f606612320f609b3568ab1dd7395f9f46306a81cf639e67289f84cefb20d371694e678794842b6163bd8f7"
        "cf8d26bbc32b78ce60da7a37c57a1764ff4632febb4dbc037ae2829375e236443ca64b0339ad834f3eeda05172"
        "f7d580a6d3cbc82939699da95b91a927f71a85010802128001b4c563d15c1333b69f5684172668539f08794cb2"
        "311e60ae820529ee2efb877ef12ba1b1a42d24cd58e3ef5e021b5a0bc78e9dabaff4fe85e99c78ee4989cd40c7"
        "37cd87858df22fb12999b5cba3fb13cb03195bf79c816ad8fcf906b12ed24ffb0ef9c6dd591b83526a2d0c47bd"
        "40abb0df8104ea0765b9a70496d7d2316fcb1a850108031280016c0230cdc6db0b7f699d1562b7dd7d0afc38aa"
        "03f1ae60d40a37ad372b3bd1b90f08260566a9f2fed3d6d100aa3fc7f4fd4654da0663c7dd6fabeb82767a902f"
        "75dd6eb3679fce6944e808304eb79a5e0e37eea7a530d4f59f4ba7edfab79864fd91988ce82ffc1c6be84712b8"
        "1bd29325fb247af2f6698d9c180fc36a07344a");
    auto decodedBlockHeader = std::make_shared<PBBlockHeader>(cryptoSuite, *data);
    BOOST_CHECK(decodedBlockHeader->version() == 10);
    BOOST_CHECK(decodedBlockHeader->txsRoot().hex() ==
                "bcd22fd79b459e46e3fbe2d1c63eb0421ef14479c5f187fa1230459e4052215d");
    BOOST_CHECK(decodedBlockHeader->receiptRoot().hex() ==
                "36433bdcdf6144fa6abee2ffccf57897f633a1f45e0f82e7796f25fed1b2f94b");
    BOOST_CHECK(decodedBlockHeader->stateRoot().hex() ==
                "0f000c0d3cf849632ed9aa2925251c0a13ee4f09f991b30bf396c15bf7ae8097");
    BOOST_CHECK(decodedBlockHeader->number() == 12312312412);
    BOOST_CHECK(decodedBlockHeader->gasUsed() == 3453456346534);
    BOOST_CHECK(decodedBlockHeader->timestamp() == 9234234234);
    BOOST_CHECK(decodedBlockHeader->sealer() == 100);
    BOOST_CHECK(*toHexString(decodedBlockHeader->extraData()) ==
                "0f000c0d3cf849632ed9aa2925251c0a13ee4f09f991b30bf396c15bf7ae8097");
    // check signature
    BOOST_CHECK(decodedBlockHeader->signatureList()->size() == 4);
    BOOST_CHECK(decodedBlockHeader->hash().hex() ==
                "53288d6e5ced9b1af30898ced137f04653e90c4d9c5e68f3bdb560cbe702e1ec");
    BOOST_CHECK((*decodedBlockHeader->parentInfo())[0].second.hex() ==
                "06d47b6f2f121e85160d1d8072e58843de1eed164ed526c3b56b22c2b47324a0");
    BOOST_CHECK(*toHexString(*(*decodedBlockHeader->sealerList())[0]) ==
                "676ebe6db5ab41f03a69eda2b5d168dbef1a39d45c02bcda519ddc142831c745d48232b533fdab95d0"
                "a9c4a949bfbda1654efb5a26dca4028c5fdd26988cb22b");
    BOOST_CHECK(*toHexString(*(*decodedBlockHeader->sealerList())[1]) ==
                "d8f7cf8d26bbc32b78ce60da7a37c57a1764ff4632febb4dbc037ae2829375e236443ca64b0339ad83"
                "4f3eeda05172f7d580a6d3cbc82939699da95b91a927f7");
    BOOST_CHECK(*toHexString(*(*decodedBlockHeader->sealerList())[2]) ==
                "c737cd87858df22fb12999b5cba3fb13cb03195bf79c816ad8fcf906b12ed24ffb0ef9c6dd591b8352"
                "6a2d0c47bd40abb0df8104ea0765b9a70496d7d2316fcb");
    BOOST_CHECK(*toHexString(*(*decodedBlockHeader->sealerList())[3]) ==
                "75dd6eb3679fce6944e808304eb79a5e0e37eea7a530d4f59f4ba7edfab79864fd91988ce82ffc1c6b"
                "e84712b81bd29325fb247af2f6698d9c180fc36a07344a");

    BOOST_CHECK(
        *toHexString(*(*decodedBlockHeader->signatureList())[0].second) ==
        "6b36496b3501cb68faac7ed258a1c0fd0c9a977115482720b9e322ee0a74a94ef4e4297863136eb7dd53581c8c"
        "b69bd4f134a3a2756ab3ddb38d30bf3529bbec676ebe6db5ab41f03a69eda2b5d168dbef1a39d45c02bcda519d"
        "dc142831c745d48232b533fdab95d0a9c4a949bfbda1654efb5a26dca4028c5fdd26988cb22b");
    BOOST_CHECK(
        *toHexString(*(*decodedBlockHeader->signatureList())[1].second) ==
        "95e6b6ba6516d1fe2b124fe673e33dd51dc39fce17d973f606612320f609b3568ab1dd7395f9f46306a81cf639"
        "e67289f84cefb20d371694e678794842b6163bd8f7cf8d26bbc32b78ce60da7a37c57a1764ff4632febb4dbc03"
        "7ae2829375e236443ca64b0339ad834f3eeda05172f7d580a6d3cbc82939699da95b91a927f7");
    BOOST_CHECK(
        *toHexString(*(*decodedBlockHeader->signatureList())[2].second) ==
        "b4c563d15c1333b69f5684172668539f08794cb2311e60ae820529ee2efb877ef12ba1b1a42d24cd58e3ef5e02"
        "1b5a0bc78e9dabaff4fe85e99c78ee4989cd40c737cd87858df22fb12999b5cba3fb13cb03195bf79c816ad8fc"
        "f906b12ed24ffb0ef9c6dd591b83526a2d0c47bd40abb0df8104ea0765b9a70496d7d2316fcb");
    BOOST_CHECK(
        *toHexString(*(*decodedBlockHeader->signatureList())[3].second) ==
        "6c0230cdc6db0b7f699d1562b7dd7d0afc38aa03f1ae60d40a37ad372b3bd1b90f08260566a9f2fed3d6d100aa"
        "3fc7f4fd4654da0663c7dd6fabeb82767a902f75dd6eb3679fce6944e808304eb79a5e0e37eea7a530d4f59f4b"
        "a7edfab79864fd91988ce82ffc1c6be84712b81bd29325fb247af2f6698d9c180fc36a07344a");
}

BOOST_AUTO_TEST_SUITE_END()
}  // namespace test
}  // namespace bcos