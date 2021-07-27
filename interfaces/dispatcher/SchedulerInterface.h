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
    struct Batch
    {
        enum Status
        {
            START = 0,
            EXECUTING,
            EXECUTED,
        };

        long batchID;
        bcos::protocol::BlockNumber beginNumber;  // [
        bcos::protocol::BlockNumber endNumber;    // )
        Status status;
    };

    /**
     * @brief start a new batch for block execution
     *
     * @param beginNumber the blocknumber to begin, must be current highest blocknumber
     * @param callback result
     */
    virtual void start(bcos::protocol::BlockNumber beginNumber,
        std::function<void(const Error::ConstPtr&, bool, const Batch&)> callback) = 0;

    /**
     * @brief executeBlock
     *
     * @param batchID
     * @param blocks
     * @param verify
     * @param callback
     */
    virtual void executeBlock(long batchID,
        const gsl::span<bcos::protocol::Block::ConstPtr>& blocks, bool verify,
        std::function<void(
            const bcos::Error::ConstPtr&, const gsl::span<bcos::protocol::BlockHeader::ConstPtr>&)>
            callback) = 0;

    /**
     * @brief commit
     *
     * @param batchID
     * @param callback
     */
    virtual void commit(long batchID, std::function<void(const Error::ConstPtr&)> callback) = 0;

    /**
     * @brief revert
     *
     * @param batchID
     * @param callback
     */
    virtual void revert(long batchID, std::function<void(const Error::ConstPtr&)> callback) = 0;

    /**
     * @brief status
     * get batch status
     * @param batchID
     * @param callback
     */
    virtual void status(
        long batchID, std::function<void(const Error::ConstPtr&, const Batch&)> callback) = 0;

    /**
     * @brief batchs
     * list all batchs
     * @param callback
     */
    virtual void batchs(
        std::function<void(const Error::ConstPtr&, const gsl::span<Batch>&)> callback) = 0;

    /**
     * @brief reset
     * clear all batchs
     */
    virtual void reset(std::function<void(const Error::ConstPtr&)> callback) = 0;
};
}  // namespace dispatcher
}  // namespace bcos