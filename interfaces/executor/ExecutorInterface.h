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
 * @file ExecutorInterface.h
 * @author: xingqiangbai
 * @date: 2021-05-17
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
class ExecutorInterface : public std::enable_shared_from_this<ExecutorInterface>
{
public:
    ExecutorInterface() = default;
    virtual ~ExecutorInterface() = default;
    // virtual bytes getCode(const string& address) const = 0;
    // virtual TransactionReceipt::Ptr BlockVerifier::executeTransaction(
    //     const BlockHeader& blockHeader, Transaction::Ptr _t) = 0;
    virtual void asyncGetCode(std::shared_ptr<std::string> _address,
        std::function<void(const Error::Ptr&, const std::shared_ptr<bytes>&)> _callback) = 0;
    virtual void asyncExecuteTransaction(const protocol::Transaction::ConstPtr& _tx,
        std::function<void(const Error::Ptr&, const protocol::TransactionReceipt::ConstPtr&)>
            _callback) = 0;
    virtual void stop() = 0;
};

}  // namespace executor
}  // namespace bcos
