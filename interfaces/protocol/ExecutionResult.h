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
#include "ProtocolTypeDef.h"
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

    enum Status
    {
        FINISHED = 0,
        PAUSE
    };

    virtual Status status() const noexcept = 0;

    // for finish
    virtual bcos::bytesConstRef output() const noexcept = 0;

    // for pause
    virtual bcos::bytesConstRef to() const noexcept = 0;
    virtual bcos::bytesConstRef input() const noexcept = 0;
};

class ExecutionResultFactory
{
public:
    using Ptr = std::shared_ptr<ExecutionResultFactory>;
    using ConstPtr = std::shared_ptr<const ExecutionResultFactory>;

    virtual ~ExecutionResultFactory(){};

    virtual ExecutionResult::Ptr createExecutionResult(ExecutionResult::Status status,
        const bcos::bytesConstRef& output, const bcos::bytesConstRef& to,
        const bcos::bytesConstRef& input) = 0;
    virtual ExecutionResult::Ptr createExecutionResult(ExecutionResult::Status status,
        bcos::bytes&& output, bcos::bytes&& to, bcos::bytes&& input) = 0;
};
}  // namespace protocol
}  // namespace bcos