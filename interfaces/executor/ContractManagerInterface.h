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
 * @brief interface of Manager
 * @file ParallelManagerInterface.h
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
class ContractManagerInterface
{
public:
    using Ptr = std::shared_ptr<ContractManagerInterface>;
    using ConstPtr = std::shared_ptr<const ContractManagerInterface>;

    struct SystemStatus
    {
        using Ptr = std::shared_ptr<SystemStatus>;
        using ConstPtr = std::shared_ptr<const SystemStatus>;

        long totalMemory;
        long memoryUsage;

        long activeContracts;
        long maxContracts;

        long loadAverage;
        long concurrency;
    };

    virtual void newExecutor(const bytesConstRef& contract,
        std::function<void(const bcos::Error::ConstPtr&)>) noexcept = 0;

    virtual void systemStatus(std::function<void(
            const bcos::Error::ConstPtr&, const SystemStatus::ConstPtr&)>) noexcept = 0;
};
}  // namespace executor
}  // namespace bcos
