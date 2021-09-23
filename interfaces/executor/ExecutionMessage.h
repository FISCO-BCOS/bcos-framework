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
 * @brief interface of ExecutionParams
 * @file ExecutionParams.h
 * @author: ancelmo
 * @date: 2021-09-22
 */

#pragma once
#include "../../libprotocol/LogEntry.h"
#include "../protocol/ProtocolTypeDef.h"
#include <memory>

namespace bcos
{
namespace protocol
{
class ExecutionMessage
{
public:
    using UniquePtr = std::unique_ptr<ExecutionMessage>;
    using UniqueConstPtr = std::unique_ptr<const ExecutionMessage>;

    enum Type
    {
        TXHASH = 0,  // Received an new transaction from scheduler
        MESSAGE,     // Send/Receive an external call to/from another contract
        FINISHED,    // Send a finish to another contract
        REVERT,      // Send/Receive a revert to / from previous external call, response
    };

    // Request fields
    virtual crypto::HashType transactionHash() const = 0;
    virtual void setTransactionHash(crypto::HashType hash) = 0;

    virtual Type type() const = 0;
    virtual void setType(Type type) = 0;

    virtual int64_t contextID() const = 0;
    virtual void setContextID(int64_t contextID) = 0;

    virtual int64_t seq() const = 0;
    virtual void setSeq(int64_t seq) = 0;

    virtual std::string_view origin() const = 0;
    virtual void setOrigin(std::string origin) = 0;

    virtual std::string_view from() const = 0;
    virtual void setFrom(std::string from) = 0;

    virtual std::string_view to() const = 0;
    virtual void setTo(std::string to) = 0;

    virtual int32_t depth() const = 0;
    virtual void setDepth(int32_t depth) = 0;

    // Request / Response fields
    virtual int64_t gasAvailable() const = 0;
    virtual void setGasAvailable(int64_t gasAvailable) = 0;

    virtual bcos::bytesConstRef data() const = 0;
    virtual bytes&& takeData() = 0;
    virtual void setData(bcos::bytes data) = 0;

    virtual bool staticCall() const = 0;
    virtual void setStaticCall(bool staticCall) = 0;

    // Response fields
    virtual int32_t status() const = 0;
    virtual void setStatus(int32_t status) = 0;

    virtual std::string_view message() const = 0;
    virtual void setMessage(std::string message) = 0;

    virtual gsl::span<LogEntry> logEntries() const = 0;
    virtual void setLogEntries(std::vector<LogEntry> logEntries) = 0;

    virtual std::optional<u256> createSalt() const = 0;
    virtual void setCreateSalt(u256 createSalt) = 0;

    virtual std::string_view newEVMContractAddress() const = 0;
    virtual void setNewEVMContractAddress(std::string newEVMContractAddress) = 0;
};

class ExecutionMessageFactory
{
public:
    using Ptr = std::shared_ptr<ExecutionMessage>;
    using ConstPtr = std::shared_ptr<const ExecutionMessage>;

    virtual ~ExecutionMessageFactory(){};

    virtual ExecutionMessage::UniquePtr createExecutionParams() = 0;
};
}  // namespace protocol
}  // namespace bcos
