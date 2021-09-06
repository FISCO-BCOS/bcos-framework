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
#include "../protocol/ProtocolTypeDef.h"
#include "../protocol/Transaction.h"
#include "../protocol/TransactionReceipt.h"
#include "ExecutionParams.h"
#include "ExecutionResult.h"
#include "TableHash.h"
#include <memory>

namespace bcos
{
namespace executor
{
class ParallelTransactionExecutorInterface
{
public:
    struct TwoPCParams
    {
        bcos::protocol::BlockNumber number;
    };

    using Ptr = std::shared_ptr<ParallelTransactionExecutorInterface>;
    using ConstPtr = std::shared_ptr<const ParallelTransactionExecutorInterface>;

    virtual void nextBlockHeader(const bcos::protocol::BlockHeader::ConstPtr& blockHeader,
        std::function<void(bcos::Error::Ptr&&)> callback) noexcept = 0;

    virtual void executeTransaction(const bcos::protocol::ExecutionParams::ConstPtr& inputs,
        std::function<void(bcos::Error::Ptr&&, bcos::protocol::ExecutionResult::Ptr&&)>
            callback) noexcept = 0;

    virtual void dagExecuteTransactions(
        const gsl::span<bcos::protocol::ExecutionParams::ConstPtr>& inputs,
        std::function<void(bcos::Error::Ptr&&, std::vector<bcos::protocol::ExecutionResult::Ptr>&&)>
            callback) noexcept = 0;

    virtual void call(const bcos::protocol::ExecutionParams::ConstPtr& input,
        std::function<void(bcos::Error::Ptr&&, bcos::protocol::ExecutionResult::Ptr&&)>
            callback) noexcept = 0;

    virtual void getTableHashes(bcos::protocol::BlockNumber number,
        std::function<void(bcos::Error::Ptr&&, std::vector<TableHash::Ptr>&&)>
            callback) noexcept = 0;

    /* ----- XA Transaction interface Start ----- */

    // Write data to storage uncommitted
    virtual void prepare(
        const TwoPCParams& params, std::function<void(bcos::Error::Ptr&&)> callback) noexcept = 0;

    // Commit uncommitted data
    virtual void commit(
        const TwoPCParams& params, std::function<void(bcos::Error::Ptr&&)> callback) noexcept = 0;

    // Rollback the changes
    virtual void rollback(
        const TwoPCParams& params, std::function<void(bcos::Error::Ptr&&)> callback) noexcept = 0;

    /* ----- XA Transaction interface End ----- */

    // drop all status
    virtual void reset(std::function<void(bcos::Error::Ptr&&)> callback) noexcept = 0;
};
}  // namespace executor
}  // namespace bcos