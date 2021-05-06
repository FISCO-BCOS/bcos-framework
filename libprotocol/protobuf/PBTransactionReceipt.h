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
 * @brief PB implementation of TransactionReceipt
 * @file PBTransactionReceipt.h
 * @author: yujiechen
 * @date: 2021-03-18
 */
#pragma once
#include "interfaces/crypto/CryptoSuite.h"
#include "interfaces/protocol/TransactionReceipt.h"
#include "libprotocol/LogEntry.h"
#include "libprotocol/TransactionStatus.h"
#include "libprotocol/bcos-proto/TransactionReceipt.pb.h"
namespace bcos
{
namespace protocol
{
class PBTransactionReceipt : public TransactionReceipt
{
public:
    PBTransactionReceipt(bcos::crypto::CryptoSuite::Ptr _cryptoSuite, bytesConstRef _receiptData);
    PBTransactionReceipt(bcos::crypto::CryptoSuite::Ptr _cryptoSuite, bytes const& _receiptData)
      : PBTransactionReceipt(_cryptoSuite, ref(_receiptData))
    {}

    PBTransactionReceipt(bcos::crypto::CryptoSuite::Ptr _cryptoSuite, int32_t _version,
        bcos::crypto::HashType const& _stateRoot, u256 const& _gasUsed,
        bytes const& _contractAddress, LogEntriesPtr _logEntries, int32_t _status,
        bytes const& _output);

    PBTransactionReceipt(bcos::crypto::CryptoSuite::Ptr _cryptoSuite, int32_t _version,
        bcos::crypto::HashType const& _stateRoot, u256 const& _gasUsed,
        bytes const& _contractAddress, LogEntriesPtr _logEntries, int32_t _status, bytes&& _output);

    ~PBTransactionReceipt() {}

    void decode(bytesConstRef _receiptData) override;
    void encode(bytes& _encodeReceiptData) override;
    bcos::crypto::HashType const& hash() override;

    int32_t version() const override { return m_receipt->version(); }
    int32_t status() const override { return m_status; }
    bytesConstRef output() const override { return ref(m_output); }
    bytesConstRef contractAddress() const override { return ref(m_contractAddress); }
    bcos::crypto::HashType const& stateRoot() const override { return m_stateRoot; }
    u256 const& gasUsed() const override { return m_gasUsed; }
    gsl::span<const LogEntry> logEntries() const override { return gsl::span<const LogEntry>(m_logEntries->data(), m_logEntries->size()); }
    LogBloom const& bloom() const override { return m_bloom; }

private:
    PBTransactionReceipt(bcos::crypto::CryptoSuite::Ptr _cryptoSuite, int32_t _version,
        bcos::crypto::HashType const& _stateRoot, u256 const& _gasUsed,
        bytes const& _contractAddress, LogEntriesPtr _logEntries, int32_t _status);
    void encodeHashFields();

private:
    bcos::crypto::CryptoSuite::Ptr m_cryptoSuite;
    std::shared_ptr<PBRawTransactionReceipt> m_receipt;

    int32_t m_version;
    bcos::crypto::HashType m_stateRoot;
    u256 m_gasUsed;
    bytes m_contractAddress;
    LogEntriesPtr m_logEntries;
    int32_t m_status;
    bytes m_output;
    LogBloom m_bloom;

    bcos::crypto::HashType m_hash;
    SharedMutex x_hash;
};
}  // namespace protocol
}  // namespace bcos