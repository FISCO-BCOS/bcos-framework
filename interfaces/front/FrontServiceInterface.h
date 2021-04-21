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
 * @brief interface for front service module
 * @file FrontInterface.h
 * @author: octopus
 * @date 2021-04-19
 */
#pragma once
#include "interfaces/crypto/KeyInterface.h"
#include "libutilities/Common.h"
#include "libutilities/Error.h"

namespace bcos
{
namespace front
{
using CallbackFunc = std::function<void(Error::Ptr, bytesConstRef)>;

/**
 * @brief: the interface provided by the front service
 */
class FrontServiceInterface
{
public:
    using Ptr = std::shared_ptr<FrontServiceInterface>;

public:
    /**
     * @brief: get nodeID list
     * @return void
     */
    virtual void asyncGetNodeIDs(std::function<void(Error::Ptr _error,
            const std::shared_ptr<const std::vector<bcos::crypto::NodeIDPtr>>&)>) const = 0;

    /**
     * @brief: send message to node
     * @param _moduleID: moduleID
     * @param _nodeID: the receiver nodeID
     * @param _data: message
     * @param _timeout: the timeout value of async function, in milliseconds.
     * @param _callback: callback
     * @return void
     */
    virtual void asyncSendMessageByNodeID(int _moduleID, bcos::crypto::NodeIDPtr _nodeID,
        bytesConstRef _data, uint32_t _timeout, CallbackFunc _callback) = 0;

    /**
     * @brief: send messages to multiple nodes
     * @param _moduleID: moduleID
     * @param _nodeIDs: the receiver nodeIDs
     * @param _data: message
     * @return void
     */
    virtual void asyncSendMessageByNodeIDs(int _moduleID,
        const std::vector<bcos::crypto::NodeIDPtr>& _nodeIDs, bytesConstRef _data) = 0;

    /**
     * @brief: send broadcast message
     * @param _moduleID: moduleID
     * @param _data:  message
     * @return void
     */
    virtual void asyncMulticastMessage(int _moduleID, bytesConstRef _data) = 0;

    /**
     * @brief: register the node change callback
     * @param _moduleID: moduleID
     * @param _callback: callback
     * @return void
     */
    virtual void registerNodeStatusNotifier(
        int _moduleID, std::function<void(Error::Ptr _error)> _callback) = 0;

    /**
     * @brief: register the callback for module message
     * @param _moduleID: moduleID
     * @param _callback: callback
     * @return void
     */
    virtual void registerMessageDispatcher(int _moduleID,
        std::function<void(Error::Ptr _error, bcos::crypto::NodeIDPtr _nodeID, bytesConstRef _data,
            std::function<void(bytesConstRef _respData)> _respFunc)>
            _callback) = 0;
};

}  // namespace front
}  // namespace bcos
