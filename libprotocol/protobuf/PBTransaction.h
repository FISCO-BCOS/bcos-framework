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
 * @brief Transaction coded in PB format
 * @file PBTransaction.h
 * @author: yujiechen
 * @date: 2021-03-16
 */
#pragma once
#include <bcos-framework/interfaces/crypto/CryptoSuite.h>
#include <bcos-framework/interfaces/protocol/Transaction.h>
#include <bcos-framework/libprotocol/bcos-proto/Transaction.pb.h>
#include <bcos-framework/libutilities/Common.h>
#include <bcos-framework/libutilities/FixedBytes.h>
#include <bcos-framework/libutilities/RefDataContainer.h>

namespace bcos
{
namespace protocol
{
class PBTransaction : public Transaction
{
public:
    using Ptr = std::shared_ptr<PBTransaction>;
    explicit PBTransaction(bcos::crypto::CryptoSuite::Ptr _cryptoSuite)
      : m_transaction(std::make_shared<PBRawTransaction>()),
        m_transactionHashFields(std::make_shared<PBRawTransactionHashFields>()),
        m_cryptoSuite(_cryptoSuite)
    {
        GOOGLE_PROTOBUF_VERIFY_VERSION;
    }
    PBTransaction(bcos::crypto::CryptoSuite::Ptr _cryptoSuite, int32_t const& _version,
        Address const& _to, bytes const& _input, u256 const& _nonce, int64_t const& _blockLimit,
        std::string const& _chainId, std::string const& _groupId, int64_t const& _importTime);

    explicit PBTransaction(
        bcos::crypto::CryptoSuite::Ptr _cryptoSuite, bytesConstRef _txData, bool _checkSig);
    explicit PBTransaction(
        bcos::crypto::CryptoSuite::Ptr _cryptoSuite, bytes const& _txData, bool _checkSig)
      : PBTransaction(_cryptoSuite, &_txData, _checkSig)
    {}

    ~PBTransaction() override {}

    bool operator==(PBTransaction const& _comparedTx)
    {
        return (m_type == _comparedTx.type()) && (m_to == to()) &&
               (importTime() == _comparedTx.importTime()) &&
               (m_transaction->hashfieldsdata() == _comparedTx.transaction()->hashfieldsdata());
    }

    void decode(bytesConstRef _txData, bool _checkSig) override;
    void encode(bytes& _txData) const override;
    h256 const& hash() const override;

    std::shared_ptr<PBRawTransaction> transaction() const { return m_transaction; }
    std::shared_ptr<PBRawTransactionHashFields> transactionHashFields() const
    {
        return m_transactionHashFields;
    }

    u256 const& nonce() const override { return m_nonce; }
    int32_t version() const override { return m_transactionHashFields->version(); }
    std::string const& chainId() const override { return m_transactionHashFields->chainid(); }
    std::string const& groupId() const override { return m_transactionHashFields->groupid(); }
    int64_t blockLimit() const override { return m_transactionHashFields->blocklimit(); }
    Address const& sender() const override { return m_sender; }
    Address const& to() const override { return m_to; }
    TransactionType const& type() const override { return m_type; }
    bytesConstRef input() const override;
    int64_t importTime() const override { return m_transaction->import_time(); }
    void forceSender(Address const& _sender) override { m_sender = _sender; }

    // only for ut
    void updateSignature(bytesConstRef _signatureData, Address const& _sender);

private:
    std::shared_ptr<PBRawTransaction> m_transaction;
    std::shared_ptr<PBRawTransactionHashFields> m_transactionHashFields;
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