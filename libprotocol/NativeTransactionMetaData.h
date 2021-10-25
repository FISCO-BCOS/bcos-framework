#pragma once

#include "../interfaces/protocol/TransactionMetaData.h"

namespace bcos::protocol
{
class NativeTransactionMetaData : public TransactionMetaData
{
public:
    using Ptr = std::shared_ptr<NativeTransactionMetaData>;
    using ConstPtr = std::shared_ptr<const NativeTransactionMetaData>;

    ~NativeTransactionMetaData() override{};

    bcos::crypto::HashType hash() const override { return m_hash; }
    std::string_view to() const override { return m_to; }

    void setHash(bcos::crypto::HashType _hash) override { m_hash = _hash; }
    void setTo(std::string _to) override { m_to = std::move(_to); }

    void setSubmitCallback(TxSubmitCallback _submitCallback) override
    {
        m_submitCallback = std::move(_submitCallback);
    }

private:
    bcos::crypto::HashType m_hash;
    std::string m_to;
    TxSubmitCallback m_submitCallback;
};

class NativeTransactionMetaDataFactory : public TransactionMetaDataFactory
{
public:
    using Ptr = std::shared_ptr<NativeTransactionMetaDataFactory>;

    ~NativeTransactionMetaDataFactory() override{};

    TransactionMetaData::Ptr createTransactionMetaData() override
    {
        return std::make_shared<NativeTransactionMetaData>();
    }
};
}  // namespace bcos::protocol