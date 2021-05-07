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
 * @brief the information of the consensus node
 * @file ConsensusNodeInterface.h
 * @author: yujiechen
 * @date 2021-04-09
 */
#pragma once
#include "../../interfaces/crypto/KeyInterface.h"
#include "../../libutilities/Log.h"
namespace bcos
{
namespace consensus
{
class ConsensusNodeInterface
{
public:
    using Ptr = std::shared_ptr<ConsensusNodeInterface>;
    ConsensusNodeInterface() = default;
    virtual ~ConsensusNodeInterface() {}

    // the nodeID of the consensus node
    virtual bcos::crypto::PublicPtr nodeID() const = 0;

    virtual uint64_t weight() const { return 100; }
};
using ConsensusNodeList = std::vector<ConsensusNodeInterface::Ptr>;
using ConsensusNodeListPtr = std::shared_ptr<ConsensusNodeList>;

struct ConsensusNodeComparator
{
    inline bool operator()(ConsensusNodeInterface::Ptr _left, ConsensusNodeInterface::Ptr _right)
    {
        if (_left->nodeID() == _right->nodeID())
        {
            return _left->weight() <= _right->weight();
        }
        return (_left->nodeID() < _right->nodeID());
    }
};

inline std::string decsConsensusNodeList(ConsensusNodeList const& _nodeList)
{
    std::ostringstream stringstream;
    for (auto node : _nodeList)
    {
        stringstream << LOG_KV(node->nodeID()->shortHex(), std::to_string(node->weight()));
    }
    return stringstream.str();
}
}  // namespace consensus
}  // namespace bcos
