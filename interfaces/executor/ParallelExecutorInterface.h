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
#include "../protocol/ProtocolTypeDef.h"
#include "../protocol/Transaction.h"
#include "../protocol/TransactionReceipt.h"
#include <memory>

namespace bcos
{
namespace executor
{
class ParallelExecutorInterface
{
public:
    enum Status {
        FINISHED,
        PAUSE
    };

    struct Args
    {
        long contextID;
        bcos::bytes input;
    };

    virtual void begin(long batchID, bcos::protocol::BlockNumber beginNumber,
        std::function<void(const Error::ConstPtr&, bool, const bcos::protocol::Batch&)>
            callback) noexcept = 0;

    virtual void end(long batchID, bool commit, bcos::protocol::BlockNumber endNumber,
        std::function<void(const Error::ConstPtr&)> callback) noexcept = 0;

    virtual void status(long batchID,
        std::function<void(const Error::ConstPtr&, const bcos::protocol::Batch&)>
            callback) noexcept = 0;

    virtual void execute(const Args& args,
        std::function<void(const bcos::Error::ConstPtr&, Status status, std::optional<Args>)>
            callback) noexcept = 0;

    virtual void dagExecute(const gsl::span<Args> argsList, std::function<void(const bcos::Error::ConstPtr&)> callback) = 0;
};
}  // namespace executor
}  // namespace bcos