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
    // batch interface
    virtual void begin(long batchID, bcos::protocol::BlockNumber beginNumber,
        std::function<void(const Error::ConstPtr&, bool, const bcos::protocol::Batch&)> callback) noexcept = 0;

    virtual void end(long batchID, bool commit, bcos::protocol::BlockNumber endNumber,
        std::function<void(const Error::ConstPtr&)> callback) noexcept = 0;

    virtual void status(long batchID,
        std::function<void(const Error::ConstPtr&, const bcos::protocol::Batch&)> callback) noexcept = 0;

    // execute interface
    virtual void executeBlock(long batchID,
        const gsl::span<bcos::protocol::Block::ConstPtr>& blocks, bool verify,
        std::function<void(
            const bcos::Error::ConstPtr&, const gsl::span<bcos::protocol::BlockHeader::ConstPtr>&)>
            callback) noexcept = 0;

    // manage interface
};
}  // namespace dispatcher
}  // namespace bcos