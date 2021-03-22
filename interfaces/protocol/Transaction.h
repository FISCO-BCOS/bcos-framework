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
 * @brief interfaces for transaction
 * @file Transaction.h
 */
#pragma once
#include <bcos-framework/libutilities/Common.h>
namespace bcos
{
namespace protocol
{
enum TransactionType
{
    NullTransaction,
    ContractCreation,
    MessageCall,
};
class Transaction
{
public:
    Transaction() = default;
    virtual ~Transaction() {}

    virtual void decode(bytesConstRef _txData, bool _checkSig) = 0;
    virtual void encode(bytes& _txData) const = 0;
    virtual h256 const& hash() const = 0;

    virtual int32_t version() const = 0;
    virtual std::string const& chainId() const = 0;
    virtual std::string const& groupId() const = 0;
    virtual int64_t blockLimit() const = 0;
    virtual u256 const& nonce() const = 0;
    virtual Address const& to() const = 0;
    virtual Address const& sender() const = 0;
    virtual bytesConstRef input() const = 0;
    virtual int64_t importTime() const = 0;
    virtual TransactionType const& type() const = 0;
    virtual void forceSender(Address const& _sender) = 0;
};
}  // namespace protocol
}  // namespace bcos