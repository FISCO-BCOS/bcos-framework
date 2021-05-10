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
 * @brief interface for BlockHeader
 * @file BlockHeader.h
 * @author: yujiechen
 * @date: 2021-03-22
 */
#pragma once
#include "ProtocolTypeDef.h"
#include <gsl/span>

namespace bcos
{
namespace protocol
{
class BlockHeader
{
public:
    using Ptr = std::shared_ptr<BlockHeader>;
    using ConstPtr = std::shared_ptr<const BlockHeader>;
    using BlockHeadersPtr = std::shared_ptr<std::vector<BlockHeader::Ptr> >;
    BlockHeader() = default;
    virtual ~BlockHeader() {}

    virtual void decode(bytesConstRef _data) = 0;
    virtual void encode(bytes& _encodeData) const = 0;
    virtual bcos::crypto::HashType const& hash() const = 0;
    virtual void populateFromParents(BlockHeadersPtr _parents, BlockNumber _number) = 0;
    virtual void clear() = 0;
    // verifySignatureList verifys the signatureList
    virtual void verifySignatureList() const = 0;
    virtual void populateEmptyBlock(int64_t _timestamp) = 0;

    // version returns the version of the blockHeader
    virtual int32_t version() const = 0;
    // parentInfo returns the parent information, including (parentBlockNumber, parentHash)
    virtual gsl::span<const ParentInfo> parentInfo() const = 0;
    // txsRoot returns the txsRoot of the current block
    virtual bcos::crypto::HashType const& txsRoot() const = 0;
    // receiptRoot returns the receiptRoot of the current block
    virtual bcos::crypto::HashType const& receiptRoot() const = 0;
    // stateRoot returns the stateRoot of the current block
    virtual bcos::crypto::HashType const& stateRoot() const = 0;
    // number returns the number of the current block
    virtual BlockNumber number() const = 0;
    virtual u256 const& gasUsed() = 0;
    virtual int64_t timestamp() = 0;
    // sealer returns the sealer that generate this block
    virtual int64_t sealer() = 0;
    // sealerList returns the current sealer list
    virtual gsl::span<const bytes> sealerList() const = 0;
    virtual bytesConstRef extraData() const = 0;
    virtual gsl::span<const Signature> signatureList() const = 0;
    virtual gsl::span<const uint64_t> consensusWeights() const = 0;

    virtual void setVersion(int32_t _version) = 0;
    virtual void setParentInfo(gsl::span<const ParentInfo> const& _parentInfo) = 0;
    virtual void setParentInfo(ParentInfoList&& _parentInfo) = 0;

    virtual void setTxsRoot(bcos::crypto::HashType const& _txsRoot) = 0;
    virtual void setReceiptRoot(bcos::crypto::HashType const& _receiptRoot) = 0;
    virtual void setStateRoot(bcos::crypto::HashType const& _stateRoot) = 0;
    virtual void setNumber(BlockNumber _blockNumber) = 0;
    virtual void setGasUsed(u256 const& _gasUsed) = 0;
    virtual void setTimestamp(int64_t _timestamp) = 0;
    virtual void setSealer(int64_t _sealerId) = 0;

    virtual void setSealerList(gsl::span<const bytes> const& _sealerList) = 0;
    virtual void setSealerList(std::vector<bytes>&& _sealerList) = 0;

    virtual void setConsensusWeights(gsl::span<const uint64_t> const& _weightList) = 0;
    virtual void setConsensusWeights(std::vector<uint64_t>&& _weightList) = 0;

    virtual void setExtraData(bytes const& _extraData) = 0;
    virtual void setExtraData(bytes&& _extraData) = 0;

    virtual void setSignatureList(gsl::span<const Signature> const& _signatureList) = 0;
    virtual void setSignatureList(SignatureList&& _signatureList) = 0;
};
}  // namespace protocol
}  // namespace bcos