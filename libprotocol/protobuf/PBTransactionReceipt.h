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
#include <bcos-framework/interfaces/crypto/CryptoSuite.h>
#include <bcos-framework/interfaces/protocol/TransactionReceipt.h>
#include <bcos-framework/libprotocol/LogEntry.h>
#include <bcos-framework/libprotocol/TransactionStatus.h>
#include <bcos-framework/libprotocol/bcos-proto/TransactionReceipt.pb.h>
namespace bcos
{
namespace protocol
{
class PBTransactionReceipt : public TransactionReceipt
{
public:
    PBTransactionReceipt(bcos::crypto::CryptoSuite::Ptr _cryptoSuite, bytes const& _receiptData);
    PBTransactionReceipt(bcos::crypto::CryptoSuite::Ptr _cryptoSuite, int32_t _version,
        h256 const& _stateRoot, u256 const& _gasUsed, Address const& _contractAddress,
        LogEntriesPtr _logEntries, TransactionStatus _status, bytes const& _output);

    PBTransactionReceipt(bcos::crypto::CryptoSuite::Ptr _cryptoSuite, int32_t _version,
        h256 const& _stateRoot, u256 const& _gasUsed, Address const& _contractAddress,
        LogEntriesPtr _logEntries, TransactionStatus _status, bytes&& _output);

    ~PBTransactionReceipt() {}

    void decode(bytes const& _receiptData) override;
    void encode(bytes& _encodeReceiptData) override;
    h256 const& hash() override;

    int32_t version() const override { return m_receipt->version(); }
    int32_t status() const override { return m_status; }
    bytesConstRef output() const override { return ref(m_output); }
    Address const& contractAddress() const override { return m_contractAddress; }
    h256 const& stateRoot() const override { return m_stateRoot; }
    u256 const& gasUsed() const override { return m_gasUsed; }
    LogEntriesPtr logEntries() const override { return m_logEntries; }
    LogBloom const& bloom() const override { return m_bloom; }

private:
    PBTransactionReceipt(bcos::crypto::CryptoSuite::Ptr _cryptoSuite, int32_t _version,
        h256 const& _stateRoot, u256 const& _gasUsed, Address const& _contractAddress,
        LogEntriesPtr _logEntries, TransactionStatus _status);
    void encodeHashFields();

private:
    bcos::crypto::CryptoSuite::Ptr m_cryptoSuite;
    std::shared_ptr<PBRawTransactionReceipt> m_receipt;

    int32_t m_version;
    h256 m_stateRoot;
    u256 m_gasUsed;
    Address m_contractAddress;
    LogEntriesPtr m_logEntries;
    int32_t m_status;
    bytes m_output;
    LogBloom m_bloom;

    h256 m_hash;
    SharedMutex x_hash;
};
}  // namespace protocol
}  // namespace bcos