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
 * @brief interface for TransactionReceipt
 * @file TransactionReceipt.h
 */
#pragma once

#include <bcos-framework/libutilities/FixedBytes.h>
namespace bcos
{
namespace protocol
{
class LogEntry;
class TransactionReceipt
{
public:
    TransactionReceipt() = default;
    virtual ~TransactionReceipt() {}

    virtual void decode(bytes const& _receiptData) = 0;
    virtual void encode(bytes& _encodedData) = 0;
    virtual h256 const& hash() = 0;

    virtual int32_t version() const = 0;
    virtual h256 const& stateRoot() const = 0;
    virtual u256 const& gasUsed() const = 0;
    virtual Address const& contractAddress() const = 0;
    virtual LogBloom const& bloom() const = 0;
    virtual int32_t status() const = 0;
    virtual bytesConstRef output() const = 0;
    virtual std::shared_ptr<std::vector<std::shared_ptr<LogEntry>>> logEntries() const = 0;
};
}  // namespace protocol
}  // namespace bcos
