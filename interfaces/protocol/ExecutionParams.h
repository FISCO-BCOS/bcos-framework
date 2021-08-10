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
 * @date: 2021-08-09
 */

#pragma once
#include "ProtocolTypeDef.h"
#include <memory>

namespace bcos
{
namespace protocol
{
class ExecutionParams
{
public:
    using Ptr = std::shared_ptr<ExecutionParams>;
    using ConstPtr = std::shared_ptr<const ExecutionParams>;

    enum Type
    {
        TXHASH = 0,     // Received an new transaction from scheduler
        EXTERN_CALL,    // Received an external call from another contract
        EXTERN_RETURN,  // Received a return value from pervious external call
    };

    virtual Type type() const = 0;
    virtual void setType(Type type) = 0;

    virtual int64_t contextID() const = 0;
    virtual void setContextID(int64_t contextID) = 0;

    virtual bcos::u256 gasAvailable() const = 0;
    virtual void setGasAvailable(bcos::u256 gasAvailable) = 0;

    virtual bcos::bytesConstRef input() const = 0;
    virtual void setInput(const bcos::bytesConstRef& input) = 0;
    virtual void setInput(bcos::bytes&& input) = 0;
};

class ExecutionParamsFactory
{
public:
    using Ptr = std::shared_ptr<ExecutionParams>;
    using ConstPtr = std::shared_ptr<const ExecutionParams>;

    virtual ~ExecutionParamsFactory() {};

    virtual ExecutionParams::Ptr createExecutionParams() = 0;
};
}  // namespace protocol
}  // namespace bcos