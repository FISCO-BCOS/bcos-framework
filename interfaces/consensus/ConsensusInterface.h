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
#include <bcos-framework/interfaces/consensus/ConsensusTypeDef.h>
#include <bcos-framework/libutilities/Error.h>

namespace bcos
{
namespace consensus
{
// Interface exposed to other modules
class ConsensusInterface
{
public:
    ConsensusInterface() = default;
    virtual ~ConsensusInterface() {}

    // get NodeID
    virtual void asyncGetNodeID(
        std::function<void(Error::Ptr, bcos::crypto::PublicPtr)> _onGetNodeID) = 0;
    // get node index
    virtual void asyncGetNodeIndex(std::function<void(Error::Ptr, IndexType)> _onGetNodeIndex) = 0;

    // get the type of the consensus node
    virtual void asyncGetNodeType(std::function<void(NodeType const&)> _onGetNodeType) = 0;

    // check the node is the sealer or not
    virtual void isLeader(std::function<void(bool)> _callback) = 0;

    // receive message from the P2P module
    // TODO: define the interface that need register to the P2P module when receiving P2P message
};
}  // namespace consensus
}  // namespace bcos
