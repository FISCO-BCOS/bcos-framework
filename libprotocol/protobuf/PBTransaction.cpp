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
 * @file PBTransaction.cpp
 * @author: yujiechen
 * @date: 2021-03-16
 */
#include "PBTransaction.h"
#include "libprotocol/Common.h"
#include "libprotocol/Exceptions.h"
using namespace bcos;
using namespace bcos::protocol;
using namespace bcos::crypto;

PBTransaction::PBTransaction(bcos::crypto::CryptoSuite::Ptr _cryptoSuite, int32_t const& _version,
    bytes const& _to, bytes const& _input, u256 const& _nonce, int64_t const& _blockLimit,
    std::string const& _chainId, std::string const& _groupId, int64_t const& _importTime)
  : PBTransaction(_cryptoSuite)
{
    m_transactionHashFields->set_version(_version);
    // set receiver address
    if (_to != bytes())
    {
        m_transactionHashFields->set_to(_to.data(), Address::size);
        m_type = TransactionType::MessageCall;
    }
    else
    {
        // TODO: After the contract path feature is implemented, this logic needs to be adjusted
        m_type = TransactionType::ContractCreation;
    }
    // set input data
    m_transactionHashFields->set_input(_input.data(), _input.size());
    // set nonce
    m_nonce = _nonce;
    bytes nonceBytes = toBigEndian(_nonce);
    m_transactionHashFields->set_nonce(nonceBytes.data(), nonceBytes.size());
    // set block limit
    m_transactionHashFields->set_blocklimit(_blockLimit);
    // set chainId
    m_transactionHashFields->set_chainid(_chainId);
    // set groupId
    m_transactionHashFields->set_groupid(_groupId);
    // encode hashFields
    auto encodedHashFieldsData = encodePBObject(m_transactionHashFields);
    m_transaction->set_hashfieldsdata(encodedHashFieldsData->data(), encodedHashFieldsData->size());
    // set import time
    m_transaction->set_import_time(_importTime);
}

PBTransaction::PBTransaction(CryptoSuite::Ptr _cryptoSuite, bytesConstRef _txData, bool _checkSig)
  : PBTransaction(_cryptoSuite)
{
    decode(_txData, _checkSig);
}

void PBTransaction::decode(bytesConstRef _txData, bool _checkSig)
{
    // decode transaction
    decodePBObject(m_transaction, _txData);
    // decode transactionHashFields
    auto transactionHashFields = m_transaction->mutable_hashfieldsdata();
    decodePBObject(m_transactionHashFields,
        bytesConstRef((byte const*)transactionHashFields->c_str(), transactionHashFields->size()));
    if (m_transactionHashFields->mutable_to()->size() > 0)
    {
        m_type = TransactionType::MessageCall;
    }
    else
    {
        m_type = TransactionType::ContractCreation;
    }
    m_nonce =
        fromBigEndian<u256>(bytesConstRef((const byte*)m_transactionHashFields->nonce().data(),
            m_transactionHashFields->nonce().size()));
    if (!_checkSig)
    {
        return;
    }
    // check the signatures
    auto signaturePtr = m_transaction->mutable_signaturedata();
    auto publicKey = m_cryptoSuite->signatureImpl()->recover(
        hash(), bytesConstRef((const byte*)signaturePtr->data(), signaturePtr->size()));
    // recover the sender
    m_sender = m_cryptoSuite->calculateAddress(publicKey).asBytes();
}

void PBTransaction::verify() const
{
    // check the signatures
    auto signaturePtr = m_transaction->mutable_signaturedata();
    auto publicKey = m_cryptoSuite->signatureImpl()->recover(
        hash(), bytesConstRef((const byte*)signaturePtr->data(), signaturePtr->size()));
    // recover the sender
    m_sender = m_cryptoSuite->calculateAddress(publicKey).asBytes();
}

void PBTransaction::encode(bytes& _txData) const
{
    auto encodedData = encodePBObject(m_transaction);
    _txData = *encodedData;
}

HashType const& PBTransaction::hash() const
{
    UpgradableGuard l(x_hash);
    if (m_hash != HashType())
    {
        return m_hash;
    }
    UpgradeGuard ul(l);
    m_hash = m_cryptoSuite->hash(m_transaction->hashfieldsdata());
    return m_hash;
}

void PBTransaction::updateSignature(bytesConstRef _signatureData, bytes const& _sender)
{
    m_transaction->set_signaturedata(_signatureData.data(), _signatureData.size());
    m_sender = _sender;
}

bytesConstRef PBTransaction::input() const
{
    auto const& inputData = m_transactionHashFields->input();
    return bytesConstRef((byte const*)(inputData.data()), inputData.size());
}