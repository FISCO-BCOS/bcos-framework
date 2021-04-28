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
 * @file PBBlockHeader.h
 * @author: yujiechen
 * @date: 2021-03-22
 */
#pragma once
#include "interfaces/crypto/CryptoSuite.h"
#include "interfaces/protocol/BlockHeader.h"
#include "libprotocol/bcos-proto/BlockHeader.pb.h"
#include "libutilities/FixedBytes.h"

namespace bcos
{
namespace protocol
{
class PBBlockHeader : public BlockHeader
{
public:
    using Ptr = std::shared_ptr<PBBlockHeader>;
    explicit PBBlockHeader(bcos::crypto::CryptoSuite::Ptr _cryptoSuite)
      : m_cryptoSuite(_cryptoSuite),
        m_blockHeader(std::make_shared<PBRawBlockHeader>())
    {}
    PBBlockHeader(bcos::crypto::CryptoSuite::Ptr _cryptoSuite, bytesConstRef _data)
      : PBBlockHeader(_cryptoSuite)
    {
        decode(_data);
    }

    PBBlockHeader(bcos::crypto::CryptoSuite::Ptr _cryptoSuite, bytes const& _data)
      : PBBlockHeader(_cryptoSuite, &_data)
    {}

    ~PBBlockHeader() override {}

    void decode(bytesConstRef _data) override;
    void encode(bytes& _encodeData) const override;
    bcos::crypto::HashType hash() const override;
    void populateFromParents(BlockHeadersPtr _parents, BlockNumber _number) override;
    void clear() override;

    // the version of the blockHeader
    int32_t version() const override { return m_blockHeader->version(); }
    // the parent information, including (parentBlockNumber, parentHash)
    gsl::span<const ParentInfo> parentInfo() const override { return m_parentInfo; }
    // the txsRoot of the current block
    bcos::crypto::HashType txsRoot() const override { return m_txsRoot; }
    // the receiptRoot of the current block
    bcos::crypto::HashType receiptRoot() const override { return m_receiptRoot; }
    // the stateRoot of the current block
    bcos::crypto::HashType stateRoot() const override { return m_stateRoot; }
    // the number of the current block
    BlockNumber number() const override { return m_number; }
    u256 gasUsed() override { return m_gasUsed; }
    int64_t timestamp() override { return m_timestamp; }
    // the sealer that generate this block
    int64_t sealer() override { return m_sealer; }
    // the current sealer list
    gsl::span<const bytes> sealerList() const override { return m_sealerList; }
    bytesConstRef extraData() const override { return bytesConstRef(m_extraData.data(), m_extraData.size()); }
    gsl::span<const Signature> signatureList() const override { return m_signatureList; }

    void verifySignatureList() const override;

    void setVersion(int32_t _version) override { m_blockHeader->set_version(_version); }
    void setParentInfo(gsl::span<const ParentInfo> const& _parentInfo) override
    {
        m_parentInfo.assign(_parentInfo.begin(), _parentInfo.end());
        noteDirty();
    }
    void setParentInfo(ParentInfoList &&_parentInfo) override {
        m_parentInfo = std::move(_parentInfo);
        noteDirty();
    }

    void setTxsRoot(bcos::crypto::HashType const& _txsRoot) override
    {
        m_txsRoot = _txsRoot;
        noteDirty();
    }

    void setReceiptRoot(bcos::crypto::HashType const& _receiptRoot) override
    {
        m_receiptRoot = _receiptRoot;
        noteDirty();
    }
    void setStateRoot(bcos::crypto::HashType const& _stateRoot) override
    {
        m_stateRoot = _stateRoot;
        noteDirty();
    }
    void setNumber(BlockNumber _blockNumber) override
    {
        m_number = _blockNumber;
        noteDirty();
    }
    void setGasUsed(u256 const& _gasUsed) override
    {
        m_gasUsed = _gasUsed;
        noteDirty();
    }
    void setTimestamp(int64_t _timestamp) override
    {
        m_timestamp = _timestamp;
        noteDirty();
    }
    void setSealer(int64_t _sealerId) override
    {
        m_sealer = _sealerId;
        noteDirty();
    }
    void setSealerList(gsl::span<const bytes> const& _sealerList) override
    {
        m_sealerList.assign(_sealerList.begin(), _sealerList.end());
        noteDirty();
    }
    void setSealerList(std::vector<bytes>&& _sealerList) override {
        m_sealerList = std::move(_sealerList);
        noteDirty();
    }
    void setExtraData(bytes const& _extraData) override
    {
        m_extraData = _extraData;
        noteDirty();
    }
    void setExtraData(bytes&& _extraData) override
    {
        m_extraData = std::move(_extraData);
        noteDirty();
    }
    void setSignatureList(gsl::span<const Signature> const& _signatureList) override
    {
        m_signatureList.assign(_signatureList.begin(), _signatureList.end());
        noteSignatureDirty();
    }
    void setSignatureList(SignatureList&&  _signatureList) override
    {
        m_signatureList = std::move(_signatureList);
        noteSignatureDirty();
    }

    void populateEmptyBlock(int64_t _timestamp) override
    {
        m_number = 0;
        m_gasUsed = 0;
        m_sealer = 0;
        m_timestamp = _timestamp;
        noteDirty();
    }

    gsl::span<const uint64_t> consensusWeights() const override { return m_consensusWeights; }

    void setConsensusWeights(gsl::span<const uint64_t> const& _weightList) override
    {
        m_consensusWeights.assign(_weightList.begin(), _weightList.end());
        noteDirty();
    }

    void setConsensusWeights(std::vector<uint64_t>&& _weightList) override {
        m_consensusWeights = std::move(_weightList);
        noteDirty();
    }

private:
    void encodeHashFields() const;
    void encodeSignatureList() const;

    void noteDirty()
    {
        m_blockHeader->clear_hashfieldsdata();
        WriteGuard l(x_hash);
        m_hash = bcos::crypto::HashType();
    }

    void noteSignatureDirty() { m_blockHeader->clear_signaturelist(); }

private:
    bcos::crypto::CryptoSuite::Ptr m_cryptoSuite;
    mutable std::shared_ptr<PBRawBlockHeader> m_blockHeader;
    ParentInfoList m_parentInfo;
    bcos::crypto::HashType m_txsRoot;
    bcos::crypto::HashType m_receiptRoot;
    bcos::crypto::HashType m_stateRoot;
    BlockNumber m_number;
    u256 m_gasUsed;
    int64_t m_timestamp;
    int64_t m_sealer = InvalidSealerIndex;
    std::vector<bytes> m_sealerList;
    bytes m_extraData;

    SignatureList m_signatureList;
    mutable bcos::crypto::HashType m_hash = bcos::crypto::HashType();
    mutable SharedMutex x_hash;

    WeightList m_consensusWeights;
};
}  // namespace protocol
}  // namespace bcos