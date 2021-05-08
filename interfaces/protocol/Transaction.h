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
#include "../../libutilities/Common.h"
#include "../../libutilities/Error.h"
#include "TransactionSubmitResult.h"
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
using TxSubmitCallback =
    std::function<void(Error::Ptr, bcos::protocol::TransactionSubmitResult::Ptr)>;
class Transaction
{
public:
    using Ptr = std::shared_ptr<Transaction>;
    using ConstPtr = std::shared_ptr<const Transaction>;
    Transaction() = default;
    virtual ~Transaction() {}

    virtual void decode(bytesConstRef _txData, bool _checkSig) = 0;
    virtual void encode(bytes& _txData) const = 0;
    virtual bcos::crypto::HashType const& hash() const = 0;

    virtual int32_t version() const = 0;
    virtual std::string_view chainId() const = 0;
    virtual std::string_view groupId() const = 0;
    virtual int64_t blockLimit() const = 0;
    virtual u256 const& nonce() const = 0;
    virtual bytesConstRef to() const = 0;
    virtual bytesConstRef sender() const = 0;
    virtual bytesConstRef input() const = 0;
    virtual int64_t importTime() const = 0;
    virtual TransactionType type() const = 0;
    virtual void forceSender(bytes const& _sender) = 0;

    virtual TxSubmitCallback submitCallback() const { return m_submitCallback; }
    virtual void setSubmitCallback(TxSubmitCallback _submitCallback)
    {
        m_submitCallback = _submitCallback;
    }
    virtual bool synced() const { return m_synced; }
    virtual void setSynced(bool _synced) { m_synced = _synced; }

    virtual void sealed() const { return m_sealed; }
    virtual void setSealed(bool _sealed) { m_sealed = _sealed; }

protected:
    TxSubmitCallback m_submitCallback;
    // the tx has been synced or not
    bool m_synced = false;
    // the tx has been sealed by the leader of not
    bool m_sealed = false;
};

using Transactions = std::vector<Transaction::Ptr>;
using TransactionsPtr = std::shared_ptr<Transactions>;
using TransactionsConstPtr = std::shared_ptr<const Transactions>;

}  // namespace protocol
}  // namespace bcos