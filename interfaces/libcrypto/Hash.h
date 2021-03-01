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
 * @brief interfaces for Hash
 * @file Hash.h
 * @author: yujiechen
 * @date 2021-03-03
 */
#pragma once
#include "bcos-framework/libutilities/FixedBytes.h"
#include <memory>
namespace bcos
{
namespace crypto
{
class Hash
{
public:
    using Ptr = std::shared_ptr<Hash>;
    Hash() = default;
    virtual ~Hash() {}
    virtual h256 hash(bytesConstRef _data) = 0;
    virtual h256 emptyHash()
    {
        if (h256() == m_emptyHash)
        {
            m_emptyHash = hash(bytesConstRef());
        }
        return m_emptyHash;
    }
    virtual h256 hash(bytes const& _data)
    {
        return hash(bytesConstRef(_data.data(), _data.size()));
    }
    virtual h256 hash(std::string const& _data) { return hash(bytesConstRef(_data)); }

    template <unsigned N>
    inline h256 hash(FixedBytes<N> const& _input)
    {
        return hash(_input.ref());
    }

private:
    h256 m_emptyHash = h256();
};
}  // namespace crypto
}  // namespace bcos
