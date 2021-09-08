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
#include "../../interfaces/crypto/CryptoSuite.h"
#include "../../interfaces/crypto/Hash.h"
#include "../../interfaces/crypto/KeyInterface.h"
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
    explicit Transaction(bcos::crypto::CryptoSuite::Ptr _cryptoSuite) : m_cryptoSuite(_cryptoSuite)
    {}

    virtual ~Transaction() {}

    virtual void decode(bytesConstRef _txData) = 0;
    virtual bytesConstRef encode(bool _onlyHashFields = false) const = 0;
    virtual bcos::crypto::HashType const& hash() const = 0;

    virtual void verify() const
    {
        // The tx has already been verified
        if (!sender().empty())
        {
            return;
        }

        // check the hash
        auto hashFields = encode(true);
        auto hash = m_cryptoSuite->hash(hashFields);

        if (hash != this->hash())
        {
            throw bcos::Exception("Hash mismatch!");
        }

        // check the signatures
        auto signature = signatureData();
        auto publicKey = m_cryptoSuite->signatureImpl()->recover(this->hash(), signature);
        // recover the sender
        forceSender(m_cryptoSuite->calculateAddress(publicKey).asBytes());
    }

    virtual int32_t version() const = 0;
    virtual std::string_view chainId() const = 0;
    virtual std::string_view groupId() const = 0;
    virtual int64_t blockLimit() const = 0;
    virtual u256 const& nonce() const = 0;
    virtual std::string_view to() const = 0;
    virtual std::string_view sender() const
    {
        return std::string_view((char*)m_sender.data(), m_sender.size());
    }

    virtual bytesConstRef input() const = 0;
    virtual int64_t importTime() const = 0;
    virtual void setImportTime(int64_t _importTime) = 0;
    virtual TransactionType type() const
    {
        if (to().size() > 0)
        {
            return TransactionType::MessageCall;
        }
        return TransactionType::ContractCreation;
    }
    virtual void forceSender(bytes const& _sender) const { m_sender = _sender; }

    virtual bytesConstRef signatureData() const = 0;

    virtual TxSubmitCallback submitCallback() const { return m_submitCallback; }
    virtual void setSubmitCallback(TxSubmitCallback _submitCallback)
    {
        m_submitCallback = _submitCallback;
    }
    virtual bool synced() const { return m_synced; }
    virtual void setSynced(bool _synced) const { m_synced = _synced; }

    virtual bool sealed() const { return m_sealed; }
    virtual void setSealed(bool _sealed) const { m_sealed = _sealed; }

    virtual bool invalid() const { return m_invalid; }
    virtual void setInvalid(bool _invalid) const { m_invalid = _invalid; }

    virtual bcos::crypto::CryptoSuite::Ptr cryptoSuite() { return m_cryptoSuite; }

    virtual void appendKnownNode(bcos::crypto::NodeIDPtr _node) const
    {
        WriteGuard l(x_knownNodeList);
        m_knownNodeList.insert(_node);
    }

    virtual bool isKnownBy(bcos::crypto::NodeIDPtr _node) const
    {
        ReadGuard l(x_knownNodeList);
        return m_knownNodeList.count(_node);
    }

    virtual void setSystemTx(bool _systemTx) const { m_systemTx = _systemTx; }
    virtual bool systemTx() const { return m_systemTx; }

    virtual void setBatchId(bcos::protocol::BlockNumber _batchId) const { m_batchId = _batchId; }
    virtual bcos::protocol::BlockNumber batchId() const { return m_batchId; }

    virtual void setBatchHash(bcos::crypto::HashType const& _hash) const { m_batchHash = _hash; }
    virtual bcos::crypto::HashType const& batchHash() const { return m_batchHash; }

protected:
    mutable bcos::bytes m_sender;
    bcos::crypto::CryptoSuite::Ptr m_cryptoSuite;

    TxSubmitCallback m_submitCallback;
    // the tx has been synced or not
    mutable std::atomic_bool m_synced = {false};
    // the tx has been sealed by the leader of not
    mutable std::atomic_bool m_sealed = {false};
    // the number of proposal that the tx batched into
    mutable bcos::protocol::BlockNumber m_batchId = {-1};
    // the hash of the proposal that the tx batched into
    mutable bcos::crypto::HashType m_batchHash;

    // the tx is invalid for verify failed
    mutable std::atomic_bool m_invalid = {false};

    // the transaction is the system transaction or not
    mutable std::atomic_bool m_systemTx = {false};

    // Record the list of nodes containing the transaction and provide related query interfaces.
    mutable bcos::SharedMutex x_knownNodeList;
    // Record the node where the transaction exists
    mutable bcos::crypto::NodeIDSet m_knownNodeList;
};

using Transactions = std::vector<Transaction::Ptr>;
using TransactionsPtr = std::shared_ptr<Transactions>;
using TransactionsConstPtr = std::shared_ptr<const Transactions>;
using ConstTransactions = std::vector<Transaction::ConstPtr>;
using ConstTransactionsPtr = std::shared_ptr<ConstTransactions>;

}  // namespace protocol
}  // namespace bcos