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
 * @brief Protobuf implementation of blockHeader
 * @file PBBlockHeader.cpp
 * @author: yujiechen
 * @date: 2021-03-22
 */
#include "PBBlockHeader.h"
#include "libcodec/scale/Scale.h"
#include "libprotocol/Common.h"
#include "libprotocol/Exceptions.h"
#include <gsl/span>

using namespace bcos;
using namespace bcos::protocol;
using namespace bcos::codec::scale;

void PBBlockHeader::decode(bytesConstRef _data)
{
    decodePBObject(m_blockHeader, _data);
    // decode hashFields data
    auto hashFieldsPtr = m_blockHeader->mutable_hashfieldsdata();
    ScaleDecoderStream stream(
        gsl::span<byte const>((byte*)hashFieldsPtr->data(), hashFieldsPtr->size()));
    stream >> m_parentInfo >> m_txsRoot >> m_receiptRoot >> m_stateRoot >> m_number >> m_gasUsed >>
        m_timestamp >> m_sealer >> m_sealerList >> m_consensusWeights >> m_extraData;

    // decode signatureList
    for (int i = 0; i < m_blockHeader->signaturelist_size(); i++)
    {
        auto signatureInfo = m_blockHeader->mutable_signaturelist(i);
        auto const& signatureData = signatureInfo->signaturedata();
        m_signatureList->emplace_back(std::make_pair(
            signatureInfo->sealerindex(), std::make_shared<bytes>(signatureData.data(),
                                              signatureData.data() + signatureData.size())));
    }
}

void PBBlockHeader::encodeHashFields() const
{
    // the hash fields has not been updated
    if (m_blockHeader->hashfieldsdata().size() > 0)
    {
        return;
    }
    ScaleEncoderStream stream;
    stream << m_parentInfo << m_txsRoot << m_receiptRoot << m_stateRoot << m_number << m_gasUsed
           << m_timestamp << m_sealer << m_sealerList << m_consensusWeights << m_extraData;
    auto hashFieldsData = stream.data();
    m_blockHeader->set_hashfieldsdata(hashFieldsData.data(), hashFieldsData.size());
}

void PBBlockHeader::encodeSignatureList() const
{
    // the signature list fields has not been updated
    if (m_blockHeader->signaturelist_size() > 0)
    {
        return;
    }
    // extend signature list field
    for (size_t i = m_blockHeader->signaturelist_size(); i < (size_t)m_signatureList->size(); i++)
    {
        m_blockHeader->add_signaturelist();
    }
    // encode signatureList
    int index = 0;
    for (auto const& signatureInfo : *m_signatureList)
    {
        auto pbSignatureInfo = m_blockHeader->mutable_signaturelist(index++);
        pbSignatureInfo->set_sealerindex(signatureInfo.first);
        pbSignatureInfo->set_signaturedata(
            signatureInfo.second->data(), signatureInfo.second->size());
    }
}

void PBBlockHeader::encode(bytes& _encodedData) const
{
    encodeHashFields();
    encodeSignatureList();
    // encode the whole blockHeader
    auto blockHeaderLen = m_blockHeader->ByteSizeLong();
    _encodedData.resize(blockHeaderLen);
    auto data = encodePBObject(m_blockHeader);
    _encodedData = *data;
}

bcos::crypto::HashType const& PBBlockHeader::hash() const
{
    UpgradableGuard l(x_hash);
    if (m_hash != bcos::crypto::HashType())
    {
        return m_hash;
    }
    encodeHashFields();
    bytesConstRef hashFieldsData =
        bytesConstRef((byte const*)m_blockHeader->hashfieldsdata().data(),
            m_blockHeader->hashfieldsdata().size());
    UpgradeGuard ul(l);
    m_hash = m_cryptoSuite->hash(hashFieldsData);
    return m_hash;
}

void PBBlockHeader::populateFromParents(BlockHeadersPtr _parents, BlockNumber _number)
{
    // set parentInfo
    for (auto parentHeader : *_parents)
    {
        m_parentInfo->push_back(std::make_pair(parentHeader->number(), parentHeader->hash()));
    }
    m_number = _number;
}

void PBBlockHeader::clear()
{
    m_blockHeader->clear_hashfieldsdata();
    m_blockHeader->clear_signaturelist();
    m_parentInfo->clear();
    m_txsRoot = bcos::crypto::HashType();
    m_receiptRoot = bcos::crypto::HashType();
    m_stateRoot = bcos::crypto::HashType();
    m_number = 0;
    m_gasUsed = u256(0);
    m_sealer = InvalidSealerIndex;
    m_sealerList->clear();
    m_extraData.clear();
    m_hash = bcos::crypto::HashType();
}

void PBBlockHeader::verifySignatureList() const
{
    if (m_sealerList->size() < m_signatureList->size())
    {
        BOOST_THROW_EXCEPTION(InvalidBlockHeader()
                              << errinfo_comment("Invalid blockHeader for the size of sealerList "
                                                 "is smaller than the size of signatureList"));
    }
    for (auto signature : *m_signatureList)
    {
        auto sealerIndex = signature.first;
        auto signatureData = signature.second;
        if (!m_cryptoSuite->signatureImpl()->verify(
                (*m_sealerList)[sealerIndex], hash(), ref(*signatureData)))
        {
            BOOST_THROW_EXCEPTION(InvalidSignatureList() << errinfo_comment(
                                      "Invalid signatureList for verify failed, signatureData:" +
                                      *toHexString(*signatureData)));
        }
    }
}