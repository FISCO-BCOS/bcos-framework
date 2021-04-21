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
 * @brief interface for Gateway module
 * @file GatewayInterface.h
 * @author: octopus
 * @date 2021-04-19
 */
#pragma once
#include "interfaces/crypto/KeyInterface.h"
#include "libutilities/Common.h"
#include "libutilities/Error.h"

namespace bcos
{
namespace gateway
{
using NodeID = bcos::crypto::NodeID;
using NodeIDs = bcos::crypto::NodeIDs;
using CallbackFunc = std::function<void(Error::Ptr, bytesConstRef)>;
/**
 * @brief: A list of interfaces provided by the gateway which are called by the front service.
 */
class GatewayInterface
{
public:
    /**
     * @brief:
     * @param _groupID : groupID
     * @param _nodeID: nodeID
     * @param _messageCallback: callback
     * @return void
     */
    virtual void registerFrontMessageCallback(
        const std::string& _groupID, const NodeID& _nodeID, CallbackFunc _messageCallback);

    /**
     * @brief:
     * @param _groupID : groupID
     * @param _nodeID: nodeID
     * @param _nodeStatusCallback: callback
     * @return void
     */
    virtual void registerNodeStatusNotifier(const std::string& _groupID, const NodeID& _nodeID,
        std::function<void(Error::Ptr _error)> _nodeStatusCallback);

    /**
     * @brief: get nodeID list
     * @return void
     */
    virtual void asyncGetNodeIDs(
        std::function<void(Error::Ptr _error, const std::shared_ptr<const std::vector<NodeID> >&)>)
        const;

    /**
     * @brief: send message to a single node
     * @param _groupID: groupID
     * @param _nodeID: the receiver nodeID
     * @param _payload: message content
     * @param _options: option parameters
     * @param _callback: callback
     * @return void
     */
    virtual void asyncSendMessageByNodeID(const std::string& _groupID, const NodeID& _nodeID,
        bytesConstRef _payload, uint32_t _timeout, CallbackFunc _callback);

    /**
     * @brief: send message to multiple nodes
     * @param _groupID: groupID
     * @param _nodeIDs: the receiver nodeIDs
     * @param _payload: message content
     * @return void
     */
    virtual void asyncSendMessageByNodeIDs(
        const std::string& _groupID, const NodeIDs& _nodeIDs, bytesConstRef _payload);

    /**
     * @brief: send message to all nodes
     * @param _groupID: groupID
     * @param _payload: message content
     * @return void
     */
    virtual void asyncMulticastMessage(const std::string& _groupID, bytesConstRef _payload);
};

}  // namespace gateway
}  // namespace bcos
