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
#include "../../libutilities/Error.h"
#include "../crypto/CommonType.h"
#include "../protocol/Block.h"
#include "interfaces/protocol/ProtocolTypeDef.h"
#include <functional>
#include <memory>

namespace bcos
{
namespace dispatcher
{
class SchedulerInterface
{
public:
    // session interfaces
    virtual void start(long sessionID, bcos::protocol::BlockNumber beginNumber,
        std::function<void(const Error::ConstPtr&, bool, const bcos::protocol::Session::ConstPtr&)>
            callback) noexcept = 0;

    virtual void commit(long sessionID, bcos::protocol::BlockNumber endNumber,
        std::function<void(const Error::ConstPtr&)> callback) noexcept = 0;

    virtual void rollback(
        long sessionID, std::function<void(const Error::ConstPtr&)> callback) noexcept = 0;

    virtual void status(long sessionID,
        std::function<void(const Error::ConstPtr&, const bcos::protocol::Session::ConstPtr&)>
            callback) noexcept = 0;

    // execute interfaces
    virtual void executeBlock(long sessionID,
        const gsl::span<bcos::protocol::Block::ConstPtr>& blocks, bool verify,
        std::function<void(const bcos::Error::ConstPtr&,
            std::shared_ptr<std::vector<bcos::protocol::BlockHeader::Ptr>>&&)>
            callback) noexcept = 0;

    virtual void callTransaction(const protocol::Transaction::ConstPtr& tx,
        std::function<void(
            const Error::ConstPtr&, protocol::TransactionReceipt::Ptr&&)>) noexcept = 0;

    // manage interfaces
    virtual void registerParallelExecutor(const bytesConstRef& contract,
        std::function<void(const Error::ConstPtr&)> callback) noexcept = 0;
};
}  // namespace dispatcher
}  // namespace bcos