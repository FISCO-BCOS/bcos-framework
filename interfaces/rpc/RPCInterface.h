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
 * @file RPCInterface.h
 * @author: octopus
 * @date 2021-07-01
 */
#pragma once

#include "../../interfaces/multigroup/GroupManagerInterface.h"
#include "../../interfaces/protocol/ProtocolTypeDef.h"
#include "../../libutilities/Error.h"

namespace bcos
{
namespace rpc
{
class RPCInterface
{
public:
    using Ptr = std::shared_ptr<RPCInterface>;

    virtual ~RPCInterface() {}

public:
    virtual void start() = 0;
    virtual void stop() = 0;

public:
    /**
     * @brief: notify blockNumber to rpc
     * @param _blockNumber: blockNumber
     * @param _callback: resp callback
     * @return void
     */
    virtual void asyncNotifyBlockNumber(
        bcos::protocol::BlockNumber _blockNumber, std::function<void(Error::Ptr)> _callback) = 0;

    /// multi-group manager related interfaces
    /**
     * @brief receive the latest group information notification from the GroupManagerInterface
     *
     * @param _groupInfo the latest group information
     */
    virtual void asyncNotifyGroupInfo(
        bcos::group::GroupInfo::Ptr _groupInfo, std::function<void(Error::Ptr&&)>) = 0;
};
}  // namespace rpc
}  // namespace bcos
