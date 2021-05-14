/**
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
 * @brief interface for Consensus module
 * @file ConsensusInterface.h
 * @author: yujiechen
 * @date 2021-04-08
 */
#pragma once
#include "interfaces/consensus/ConsensusTypeDef.h"
#include "interfaces/crypto/KeyInterface.h"
#include "libutilities/Error.h"

namespace bcos
{
namespace consensus
{
// ConsensusInterface is the interface of consensus exposed to other modules
class ConsensusInterface
{
public:
    ConsensusInterface() = default;
    virtual ~ConsensusInterface() {}

    // asyncGetNodeID gets NodeID
    virtual void asyncGetNodeID(
        std::function<void(Error::Ptr, bcos::crypto::PublicPtr)> _onGetNodeID) = 0;
    // asyncGetNodeIndex gets node index
    virtual void asyncGetNodeIndex(std::function<void(Error::Ptr, IndexType)> _onGetNodeIndex) = 0;

    // asyncGetNodeType gets the type of the consensus node
    virtual void asyncGetNodeType(std::function<void(NodeType const&)> _onGetNodeType) = 0;

    // isLeader checks whether the node is the sealer or not
    virtual void isLeader(std::function<void(bool)> _callback) = 0;

    virtual void asyncSubmitProposal(bytesConstRef _proposalData,
        bcos::protocol::BlockNumber _proposalIndex, bcos::crypto::HashType const& _proposalHash,
        std::function<void(Error::Ptr)> _onProposalSubmitted)
};
}  // namespace consensus
}  // namespace bcos
