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
 * @brief interfaces for block sync
 * @file BlockSyncInterface.h
 * @author: yujiechen
 * @date 2021-05-23
 */

#pragma once
#include "../../interfaces/crypto/KeyInterface.h"
#include "../../interfaces/ledger/LedgerConfig.h"
#include <memory>
namespace bcos
{
namespace sync
{
class BlockSyncInterface
{
public:
    using Ptr = std::shared_ptr<BlockSyncInterface>;
    BlockSyncInterface() = default;
    virtual ~BlockSyncInterface() {}

    virtual void start() = 0;
    virtual void stop() = 0;

    // called by the frontService to dispatch message
    virtual void asyncNotifyBlockSyncMessage(Error::Ptr _error, std::string const& _uuid,
        bcos::crypto::NodeIDPtr _nodeID, bytesConstRef _data,
        std::function<void(Error::Ptr _error)> _onRecv) = 0;
};
}  // namespace sync
}  // namespace bcos