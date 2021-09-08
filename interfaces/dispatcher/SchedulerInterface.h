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
 * @brief interface of Scheduler
 * @file SchedulerInterface.h
 * @author: ancelmo
 * @date: 2021-07-27
 */

#pragma once
#include "../../interfaces/executor/ParallelTransactionExecutorInterface.h"
#include "../../interfaces/ledger/LedgerConfig.h"
#include "../../interfaces/protocol/ProtocolTypeDef.h"
#include "../../libutilities/Error.h"
#include "../crypto/CommonType.h"
#include "../protocol/Block.h"
#include <functional>
#include <memory>
#include <string_view>

namespace bcos::dispatcher
{
class SchedulerInterface
{
public:
    using Ptr = std::shared_ptr<SchedulerInterface>;
    SchedulerInterface() = default;
    virtual ~SchedulerInterface() {}

    // by pbft & sync
    virtual void executeBlock(const bcos::protocol::Block::ConstPtr& block, bool verify,
        std::function<void(bcos::Error::Ptr&&, bcos::protocol::BlockHeader::Ptr&&)>
            callback) noexcept = 0;

    /**
     * @brief async commit a block in consensus/sync module
     * @param _blockHeader the header to commit, this header should have signList
     * @param _onCommitBlock trigger this callback when commit block in storage
     *                       return the system ledger config to the sync/consensus module
     */
    virtual void commitBlock(const bcos::protocol::BlockHeader::ConstPtr& header,
        std::function<void(bcos::Error::Ptr&&, bcos::ledger::LedgerConfig::Ptr&&)>) noexcept = 0;

    // by console, query committed committing executing
    virtual void status(std::function<void(Error::Ptr&&, bcos::protocol::Session::ConstPtr&&)>
            callback) noexcept = 0;

    // by rpc
    virtual void call(const protocol::Transaction::ConstPtr& tx,
        std::function<void(Error::Ptr&&, protocol::TransactionReceipt::Ptr&&)>) noexcept = 0;

    // by executor
    virtual void registerExecutor(const std::string& name,
        const bcos::executor::ParallelTransactionExecutorInterface::Ptr& executor,
        std::function<void(Error::Ptr&&)> callback) noexcept = 0;

    virtual void unregisterExecutor(
        const std::string& name, std::function<void(Error::Ptr&&)> callback) noexcept = 0;

    // clear all status
    virtual void reset(std::function<void(Error::Ptr&&)> callback) noexcept = 0;
};
}  // namespace bcos::dispatcher
