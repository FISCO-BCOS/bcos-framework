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
 * @file AMOPInterface.h
 * @author: octopus
 * @date 2021-06-17
 */
#pragma once

#include <bcos-framework/interfaces/front/FrontServiceInterface.h>
#include <bcos-framework/libutilities/Common.h>

namespace bcos
{
namespace amop
{
class AMOPInterface
{
public:
    using Ptr = std::shared_ptr<AMOPInterface>;

    virtual ~AMOPInterface() = 0;

public:
    virtual void start() = 0;
    virtual void stop() = 0;

public:
    /**
     * @brief: async receive message from front service
     * @param _nodeID: the message sender nodeID
     * @param _id: the id of this message, it can by used to send response to the peer
     * @param _data: the message data
     * @return void
     */
    virtual void asyncNotifyAmopMessage(
        bcos::crypto::NodeIDPtr _nodeID, const std::string& _id, bcos::bytesConstRef _data) = 0;
    /**
     * @brief: async receive nodeIDs from front service
     * @param _nodeIDs: the nodeIDs
     * @param _callback: callback
     * @return void
     */
    virtual void asyncNotifyAmopNodeIDs(std::shared_ptr<const bcos::crypto::NodeIDs> _nodeIDs,
        std::function<void(bcos::Error::Ptr _error)> _callback) = 0;
};
}  // namespace amop
}  // namespace bcos
