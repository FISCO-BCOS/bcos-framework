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
 * @brief implementation of TransactionReceipt
 * @file PBTransactionReceipt.cpp
 * @author: yujiechen
 * @date: 2021-03-18
 */
#include "PBTransactionReceipt.h"
#include <bcos-framework/libcodec/scale/Scale.h>
#include <gsl/span>

using namespace bcos;
using namespace bcos::protocol;
using namespace bcos::crypto;
using namespace bcos::codec::scale;

PBTransactionReceipt::PBTransactionReceipt(
    bcos::crypto::CryptoSuite::Ptr _cryptoSuite, bytes const& _receiptData)
  : m_cryptoSuite(_cryptoSuite), m_receipt(std::make_shared<PBRawTransactionReceipt>())
{
    decode(_receiptData);
}

PBTransactionReceipt::PBTransactionReceipt(bcos::crypto::CryptoSuite::Ptr _cryptoSuite,
    int32_t _version, h256 const& _stateRoot, u256 const& _gasUsed, Address const& _contractAddress,
    LogEntriesPtr _logEntries, TransactionStatus _status)
  : m_cryptoSuite(_cryptoSuite),
    m_receipt(std::make_shared<PBRawTransactionReceipt>()),
    m_stateRoot(_stateRoot),
    m_gasUsed(_gasUsed),
    m_contractAddress(_contractAddress),
    m_logEntries(_logEntries),
    m_status((int32_t)_status),
    m_bloom(generateBloom(_logEntries, _cryptoSuite))
{
    m_receipt->set_version(_version);
}

PBTransactionReceipt::PBTransactionReceipt(bcos::crypto::CryptoSuite::Ptr _cryptoSuite,
    int32_t _version, h256 const& _stateRoot, u256 const& _gasUsed, Address const& _contractAddress,
    LogEntriesPtr _logEntries, TransactionStatus _status, bytes const& _ouptput)
  : PBTransactionReceipt(
        _cryptoSuite, _version, _stateRoot, _gasUsed, _contractAddress, _logEntries, _status)
{
    m_output = _ouptput;
}

PBTransactionReceipt::PBTransactionReceipt(bcos::crypto::CryptoSuite::Ptr _cryptoSuite,
    int32_t _version, h256 const& _stateRoot, u256 const& _gasUsed, Address const& _contractAddress,
    LogEntriesPtr _logEntries, TransactionStatus _status, bytes&& _ouptput)
  : PBTransactionReceipt(
        _cryptoSuite, _version, _stateRoot, _gasUsed, _contractAddress, _logEntries, _status)
{
    m_output = std::move(_ouptput);
}

void PBTransactionReceipt::decode(bytes const& _data)
{
    // decode receipt
    if (!m_receipt->ParseFromArray(_data.data(), _data.size()))
    {
        BOOST_THROW_EXCEPTION(
            ReceiptDecodeException() << errinfo_comment(
                "decode transaction receipt failed, receiptData:" + *toHexString(_data)));
    }
    ScaleDecoderStream stream(gsl::span<const byte>(
        (byte*)m_receipt->hashfieldsdata().data(), m_receipt->hashfieldsdata().size()));
    stream >> m_status >> m_output >> m_contractAddress >> m_stateRoot >> m_gasUsed >> m_bloom >>
        m_logEntries;
}

void PBTransactionReceipt::encode(bytes& _encodeReceiptData)
{
    encodeHashFields();
    auto receiptDataLen = m_receipt->ByteSizeLong();
    _encodeReceiptData.resize(receiptDataLen);
    if (!m_receipt->SerializeToArray(_encodeReceiptData.data(), receiptDataLen))
    {
        BOOST_THROW_EXCEPTION(
            ReceiptEncodeException() << errinfo_comment("encode transactionReceipt failed"));
    }
}

void PBTransactionReceipt::encodeHashFields()
{
    // the hash field has already been encoded
    if (m_receipt->hashfieldsdata().size() > 0)
    {
        return;
    }
    // encode the hashFieldsData
    ScaleEncoderStream stream;
    stream << m_status << m_output << m_contractAddress << m_stateRoot << m_gasUsed << m_bloom
           << m_logEntries;
    auto hashFieldsData = stream.data();
    m_receipt->set_version(m_version);
    m_receipt->set_hashfieldsdata(hashFieldsData.data(), hashFieldsData.size());
}

h256 const& PBTransactionReceipt::hash()
{
    UpgradableGuard l(x_hash);
    if (m_hash != h256())
    {
        return m_hash;
    }
    if (m_receipt->hashfieldsdata().size() == 0)
    {
        encodeHashFields();
    }
    bytesConstRef hashFieldsData = bytesConstRef(
        (byte const*)m_receipt->hashfieldsdata().data(), m_receipt->hashfieldsdata().size());
    UpgradeGuard ul(l);
    m_hash = m_cryptoSuite->hash(hashFieldsData);
    return m_hash;
}