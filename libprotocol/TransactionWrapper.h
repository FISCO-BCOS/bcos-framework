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
 * @brief wrapper for Transaction
 * @file TransactionWrapper.h
 * @author: yujiechen
 * @date: 2021-03-16
 */
#pragma once
#include <bcos-framework/interfaces/crypto/CryptoSuite.h>
#include <bcos-framework/libprotocol/bcos-proto/Transaction.pb.h>
#include <bcos-framework/libutilities/Common.h>
#include <bcos-framework/libutilities/FixedBytes.h>
#include <bcos-framework/libutilities/RefDataContainer.h>

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
class TransactionWrapper
{
public:
    using Ptr = std::shared_ptr<TransactionWrapper>;
    explicit TransactionWrapper(bcos::crypto::CryptoSuite::Ptr _cryptoSuite)
      : m_transaction(std::make_shared<Transaction>()),
        m_transactionHashFields(std::make_shared<TransactionHashFields>()),
        m_cryptoSuite(_cryptoSuite)
    {
        GOOGLE_PROTOBUF_VERIFY_VERSION;
    }

    TransactionWrapper(bcos::crypto::CryptoSuite::Ptr _cryptoSuite, int32_t const& _version,
        Address const& _to, bytes const& _input, u256 const& _nonce, int64_t const& _blockLimit,
        std::string const& _chainId, std::string const& _groupId, int64_t const& _importTime);

    explicit TransactionWrapper(
        bcos::crypto::CryptoSuite::Ptr _cryptoSuite, bytesConstRef _txData, bool _checkSig);
    explicit TransactionWrapper(
        bcos::crypto::CryptoSuite::Ptr _cryptoSuite, bytes const& _txData, bool _checkSig)
      : TransactionWrapper(_cryptoSuite, &_txData, _checkSig)
    {}

    virtual ~TransactionWrapper() {}

    bool operator==(TransactionWrapper const& _comparedTx)
    {
        return (m_type == _comparedTx.type()) && (m_to == to()) &&
               (m_transaction->import_time() == _comparedTx.transaction()->import_time()) &&
               (m_transaction->hashfieldsdata() == _comparedTx.transaction()->hashfieldsdata());
    }

    virtual void decode(bytesConstRef _txData, bool _checkSig);
    virtual void encode(bytes& _txData) const;
    virtual h256 const& hash() const;

    std::shared_ptr<Transaction> transaction() const { return m_transaction; }
    std::shared_ptr<TransactionHashFields> transactionHashFields() const
    {
        return m_transactionHashFields;
    }

    Address const& sender() const { return m_sender; }
    Address const& to() const { return m_to; }
    TransactionType const& type() const { return m_type; }
    u256 const& nonce() const { return m_nonce; }
    // only for ut
    void updateSignature(bytesConstRef _signatureData, Address const& _sender);

    void forceSender(Address const& _sender) { m_sender = _sender; }

private:
    std::shared_ptr<Transaction> m_transaction;
    std::shared_ptr<TransactionHashFields> m_transactionHashFields;
    bcos::crypto::CryptoSuite::Ptr m_cryptoSuite;

    mutable h256 m_hash;
    mutable SharedMutex x_hash;

    bcos::Address m_sender;
    bcos::Address m_to;
    u256 m_nonce;
    TransactionType m_type;
};
}  // namespace protocol
}  // namespace bcos
