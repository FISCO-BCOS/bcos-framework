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
 * @file TransactionWrapper.cpp
 * @author: yujiechen
 * @date: 2021-03-16
 */
#include "TransactionWrapper.h"
#include "Exceptions.h"
using namespace bcos;
using namespace bcos::protocol;
using namespace bcos::crypto;

TransactionWrapper::TransactionWrapper(bcos::crypto::CryptoSuite::Ptr _cryptoSuite,
    int32_t const& _version, Address const& _to, bytes const& _input, u256 const& _nonce,
    int64_t const& _blockLimit, std::string const& _chainId, std::string const& _groupId,
    int64_t const& _importTime)
  : TransactionWrapper(_cryptoSuite)
{
    m_transactionHashFields->set_version(_version);
    // set receiver address
    m_to = _to;
    if (_to != Address())
    {
        m_transactionHashFields->set_to(_to.data(), Address::size);
        m_type = TransactionType::MessageCall;
    }
    else
    {
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
    std::shared_ptr<bytes> encodedHashFieldsData = std::make_shared<bytes>();
    encodedHashFieldsData->resize(m_transactionHashFields->ByteSizeLong());
    m_transactionHashFields->SerializeToArray(
        encodedHashFieldsData->data(), encodedHashFieldsData->size());
    m_transaction->set_hashfieldsdata(encodedHashFieldsData->data(), encodedHashFieldsData->size());
    // set import time
    m_transaction->set_import_time(_importTime);
}

TransactionWrapper::TransactionWrapper(
    CryptoSuite::Ptr _cryptoSuite, bytesConstRef _txData, bool _checkSig)
  : TransactionWrapper(_cryptoSuite)
{
    decode(_txData, _checkSig);
}

void TransactionWrapper::decode(bytesConstRef _txData, bool _checkSig)
{
    // decode transaction
    if (!m_transaction->ParseFromArray((const void*)_txData.data(), _txData.size()))
    {
        BOOST_THROW_EXCEPTION(TransactionDecodeException() << errinfo_comment(
                                  "decode transaction failed, txData:" + *toHexString(_txData)));
    }
    // decode transactionHashFields
    auto transactionHashFields = m_transaction->mutable_hashfieldsdata();
    if (!m_transactionHashFields->ParseFromArray(
            (const void*)transactionHashFields->data(), transactionHashFields->size()))
    {
        BOOST_THROW_EXCEPTION(
            TransactionDecodeException() << errinfo_comment(
                "decode transaction hash fields failed, txData:" + *toHexString(_txData)));
    }
    if (!_checkSig)
    {
        return;
    }
    // check the signatures
    auto signaturePtr = m_transaction->mutable_siganturedata();
    auto publicKey = m_cryptoSuite->signatureImpl()->recover(
        hash(), bytesConstRef((const byte*)signaturePtr->data(), signaturePtr->size()));
    // recover the sender
    m_sender = m_cryptoSuite->signatureImpl()->calculateAddress(publicKey);
    if (m_transactionHashFields->mutable_to()->size() > 0)
    {
        m_to = Address((byte const*)m_transactionHashFields->mutable_to()->data(),
            Address::ConstructorType::FromPointer);
        m_type = TransactionType::MessageCall;
    }
    else
    {
        m_type = TransactionType::ContractCreation;
    }
    m_nonce =
        fromBigEndian<u256>(bytesConstRef((const byte*)m_transactionHashFields->nonce().data(),
            m_transactionHashFields->nonce().size()));
}

void TransactionWrapper::encode(bytes& _txData) const
{
    auto txSize = m_transaction->ByteSizeLong();
    _txData.resize(txSize);
    auto success = m_transaction->SerializeToArray(_txData.data(), txSize);
    if (!success)
    {
        BOOST_THROW_EXCEPTION(
            TransactionEncodeException() << errinfo_comment("encode transaction failed"));
    }
}

h256 const& TransactionWrapper::hash() const
{
    UpgradableGuard l(x_hash);
    if (m_hash != h256())
    {
        return m_hash;
    }
    UpgradeGuard ul(l);
    m_hash = m_cryptoSuite->hash(m_transaction->hashfieldsdata());
    return m_hash;
}

void TransactionWrapper::updateSignature(bytesConstRef _signatureData, Address const& _sender)
{
    m_transaction->set_siganturedata(_signatureData.data(), _signatureData.size());
    m_sender = _sender;
}