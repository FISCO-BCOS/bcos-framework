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
 * @brief interface of ExecutionResult
 * @file ExecutionResult.h
 * @author: ancelmo
 * @date: 2021-07-28
 */

#pragma once
#include "../../libprotocol/LogEntry.h"
#include "../protocol/ProtocolTypeDef.h"
#include "../protocol/TransactionReceipt.h"
#include <memory>

namespace bcos
{
namespace protocol
{
class ExecutionResult
{
public:
    using Ptr = std::shared_ptr<ExecutionResult>;
    using ConstPtr = std::shared_ptr<const ExecutionResult>;

    virtual ~ExecutionResult(){};

    enum Type
    {
        FINISHED = 0,   // Finish the execution, whether from scheduler or another contract
        EXTERNAL_CALL,  // Generate an external call
        SEND_BACK,      // The transaction cannot be executed concurrently via DAG mechanism
    };

    virtual Type type() const = 0;
    virtual void setType(Type type) = 0;

    // Only when type is EXTERNAL_CALL, it is not 0
    virtual int64_t contextID() const = 0;
    virtual void setContextID(int64_t contextID) = 0;

    virtual int64_t gasAvailable() const = 0;
    virtual void setGasAvailable(int64_t gasAvailable) = 0;

    virtual int32_t status() const = 0;
    virtual void setStatus(int32_t status) = 0;

    virtual std::string_view message() const = 0;
    virtual void setMessage(std::string message) = 0;

    // When the type is FINISH, it means that the transaction is executed and output is the return
    // value of the transaction. When the type is EXTERNAL_CALL, output is the input parameter of
    // the next call
    virtual bcos::bytesConstRef output() const = 0;
    virtual void setOutput(bytes output) = 0;

    virtual gsl::span<LogEntry> logEntries() const = 0;
    virtual void setLogEntries(LogEntriesPtr logEntries) = 0;

    // Only when status is EXTERNAL_CALL, it is not empty
    virtual std::string_view to() const = 0;
    virtual void setTo(std::string to) = 0;

    // For evm
    virtual std::string_view newEVMContractAddress() const = 0;
    virtual void setNewEVMContractAddress(std::string newEVMContractAddress) = 0;

    // for solidity
    virtual std::optional<u256> createSalt() const = 0;
    virtual void setCreateSalt(u256 createSalt) = 0;

    // for evm
    virtual bool staticCall() const = 0;
    virtual void setStaticCall(bool staticCall) = 0;
};

class ExecutionResultFactory
{
public:
    using Ptr = std::shared_ptr<ExecutionResultFactory>;
    using ConstPtr = std::shared_ptr<const ExecutionResultFactory>;

    virtual ~ExecutionResultFactory(){};

    virtual ExecutionResult::Ptr createExecutionResult() = 0;
};
}  // namespace protocol
}  // namespace bcos
