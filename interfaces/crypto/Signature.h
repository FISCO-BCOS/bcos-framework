/**
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
 * @brief interfaces for Signature
 * @file Signature.h
 * @author: yujiechen
 * @date 2021-03-03
 */
#pragma once
#include <bcos-framework/libutilities/FixedBytes.h>
#include <memory>
namespace bcos
{
namespace crypto
{
using Secret = SecureFixedBytes<32>;
using Public = h512;
class KeyPair
{
public:
    using Ptr = std::shared_ptr<KeyPair>;
    KeyPair() = default;
    KeyPair(Secret const& _secretKey, Public const& _publicKey)
      : m_secretKey(_secretKey), m_publicKey(_publicKey)
    {}
    KeyPair(KeyPair const& _keyPair) : KeyPair(_keyPair.secretKey(), _keyPair.publicKey()) {}
    virtual ~KeyPair() {}

    Secret const& secretKey() const { return m_secretKey; }
    Public const& publicKey() const { return m_publicKey; }

    virtual Address const& address()
    {
        if (Address() == m_address)
        {
            m_address = calculateAddress(m_publicKey);
        }
        return m_address;
    }

    virtual Address calculateAddressBySecret(Secret const& _secret)
    {
        return calculateAddress(priToPub(_secret));
    }

    virtual Address calculateAddress(Public const& _pub) = 0;
    virtual Public priToPub(Secret const& _secret) = 0;

    void setSecretKey(Secret const& _secretKey) { m_secretKey = _secretKey; }
    void setPublicKey(Public const& _publicKey) { m_publicKey = _publicKey; }

protected:
    Secret m_secretKey;
    Public m_publicKey;
    Address m_address = Address();
};

class SignatureData
{
public:
    using Ptr = std::shared_ptr<SignatureData>;
    SignatureData() = default;
    SignatureData(h256 const& _r, h256 const& _s) : m_r(_r), m_s(_s) {}
    virtual ~SignatureData() {}
    virtual void encode(bytesPointer _signatureData) const = 0;
    virtual void decode(bytes const& _signatureData) = 0;

    h256 const& r() const { return m_r; }
    h256 const& s() const { return m_s; }

protected:
    virtual void decodeCommonFields(bytes const& _signatureData)
    {
        if (_signatureData.size() < m_signatureLen)
        {
            BOOST_THROW_EXCEPTION(
                InvalidSignatureData() << errinfo_comment(
                    "invalid InvalidSignatureData: the signature data size must be at least " +
                    std::to_string(m_signatureLen)));
        }
        m_r = h256(_signatureData.data(), h256::ConstructorType::FromPointer);
        m_s = h256(_signatureData.data() + 32, h256::ConstructorType::FromPointer);
    }
    virtual void encodeCommonFields(bytesPointer _signatureData) const
    {
        _signatureData->resize(m_signatureLen);
        memcpy(_signatureData->data(), m_r.data(), 32);
        memcpy(_signatureData->data() + 32, m_s.data(), 32);
    }

protected:
    size_t m_signatureLen;

private:
    h256 m_r;
    h256 m_s;
};

class SignatureCrypto
{
public:
    using Ptr = std::shared_ptr<SignatureCrypto>;
    SignatureCrypto() = default;
    virtual ~SignatureCrypto() {}
    virtual std::shared_ptr<bytes> sign(KeyPair const& _keyPair, const h256& _hash) = 0;
    virtual bool verify(
        Public const& _pubKey, const h256& _hash, std::shared_ptr<bytes> _signatureData) = 0;
    // recover the public key from the given signature
    virtual Public recover(const h256& _hash, std::shared_ptr<bytes> _signatureData) = 0;
    virtual std::shared_ptr<KeyPair> generateKeyPair() = 0;

    virtual bool verify(Public const& _pubKey, const h256& _hash, SignatureData::Ptr _signatureData)
    {
        auto signatureRawData = std::make_shared<bytes>();
        _signatureData->encode(signatureRawData);
        return verify(_pubKey, _hash, signatureRawData);
    }

    Public recoverSignature(const h256& _hash, SignatureData::Ptr _signatureData)
    {
        auto signatureRawData = std::make_shared<bytes>();
        _signatureData->encode(signatureRawData);
        return recover(_hash, signatureRawData);
    }
};
}  // namespace crypto
}  // namespace bcos
