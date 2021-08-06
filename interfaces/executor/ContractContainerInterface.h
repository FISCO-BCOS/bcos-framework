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
 * @brief interface of Executor
 * @file ParallelExecutorInterface.h
 * @author: ancelmo
 * @date: 2021-07-27
 */

#pragma once

#include "../../libutilities/Common.h"
#include "../../libutilities/FixedBytes.h"
#include "../crypto/CommonType.h"
#include "../protocol/BlockHeader.h"
#include "../protocol/ExecutionResult.h"
#include "../protocol/ProtocolTypeDef.h"
#include "../protocol/Transaction.h"
#include "../protocol/TransactionReceipt.h"
#include <memory>

namespace bcos
{
namespace executor
{
class ContractContainerInterface
{
public:
    using Ptr = std::shared_ptr<ContractContainerInterface>;
    using ConstPtr = std::shared_ptr<const ContractContainerInterface>;

    // Session interfaces
    virtual void start(int64_t sessionID, bcos::protocol::BlockNumber beginNumber,
        std::function<void(const Error::ConstPtr&, bool, const bcos::protocol::Session::ConstPtr&)>
            callback) noexcept = 0;

    virtual void commit(int64_t sessionID, bcos::protocol::BlockNumber endNumber,
        std::function<void(const Error::ConstPtr&)> callback) noexcept = 0;

    virtual void rollback(
        int64_t sessionID, std::function<void(const Error::ConstPtr&)> callback) noexcept = 0;

    virtual void status(int64_t sessionID,
        std::function<void(const Error::ConstPtr&, const bcos::protocol::Session::ConstPtr&)>
            callback) noexcept = 0;

    // Execute interfaces
    virtual void setBlockHeader(const bcos::protocol::BlockHeader::ConstPtr& blockHeader,
        std::function<void(const bcos::Error::ConstPtr&)> callback);

    virtual void executeByTxHash(int64_t sessionID, const bcos::crypto::HashType& txHash,
        std::function<void(const bcos::Error::ConstPtr&, bcos::protocol::ExecutionResult::Ptr&&)>
            callback) noexcept = 0;

    // When the contextID is equal to 0, it means that the transaction will be executed from an
    // empty context, When the contextID is not equal to 0, it means to continue to execute an
    // existing context, and the input parameter is the return value of the previous call
    virtual void executeByInput(int64_t sessionID, int64_t contextID, const bytesConstRef& input,
        std::function<void(const bcos::Error::ConstPtr&, bcos::protocol::ExecutionResult::Ptr&&)>
            callback) noexcept = 0;

    // Manage interfaces
    virtual void reset(std::function<void(const bcos::Error::ConstPtr&)>) noexcept = 0;

    virtual void shutdown(std::function<void(const bcos::Error::ConstPtr&)> callback) noexcept = 0;
};
}  // namespace executor
}  // namespace bcos
