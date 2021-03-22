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
 * @file LogEntry.h
 * @author: yujiechen
 * @date: 2021-03-18
 */
#pragma once
#include <bcos-framework/interfaces/crypto/CryptoSuite.h>
#include <bcos-framework/libutilities/Common.h>
#include <bcos-framework/libutilities/FixedBytes.h>
namespace bcos
{
namespace protocol
{
class LogEntry
{
public:
    using Ptr = std::shared_ptr<LogEntry>;
    LogEntry() = default;
    LogEntry(Address const& _address, h256s _topics, bytes _data)
      : m_address(_address), m_topics(std::move(_topics)), m_data(std::move(_data))
    {}

    virtual ~LogEntry() {}

    virtual Address const& address() const { return m_address; }
    virtual h256s const& topics() const { return m_topics; }
    virtual bytes const& data() const { return m_data; }

    virtual LogBloom bloom(bcos::crypto::CryptoSuite::Ptr _cryptoSuite) const
    {
        LogBloom logBloom;
        logBloom.shiftBloom<3>(_cryptoSuite->hash(m_address.ref()));
        for (auto const& topic : m_topics)
        {
            logBloom.shiftBloom<3>(_cryptoSuite->hash(topic.ref()));
        }
        return logBloom;
    }

    // Define the scale decode method, which cannot be modified at will
    template <class Stream, typename = std::enable_if_t<Stream::is_decoder_stream>>
    friend Stream& operator>>(Stream& _stream, LogEntry& _logEntry)
    {
        return _stream >> _logEntry.m_address >> _logEntry.m_topics >> _logEntry.m_data;
    }

    // Define the scale encode method, which cannot be modified at will
    template <class Stream, typename = std::enable_if_t<Stream::is_encoder_stream>>
    friend Stream& operator<<(Stream& _stream, LogEntry const& _logEntry)
    {
        return _stream << _logEntry.address() << _logEntry.topics() << _logEntry.data();
    }

private:
    bcos::Address m_address;
    bcos::h256s m_topics;
    bytes m_data;
};

using LogEntries = std::vector<LogEntry::Ptr>;
using LogEntriesPtr = std::shared_ptr<std::vector<LogEntry::Ptr>>;

inline LogBloom generateBloom(
    LogEntriesPtr _logEntries, bcos::crypto::CryptoSuite::Ptr _cryptoSuite)
{
    LogBloom logBloom;
    for (auto const& logEntry : *_logEntries)
    {
        logBloom |= logEntry->bloom(_cryptoSuite);
    }
    return logBloom;
}
}  // namespace protocol
}  // namespace bcos