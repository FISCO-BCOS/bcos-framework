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
 * @brief fake sealer
 * @file FakeSealer.h
 * @author: yujiechen
 * @date 2021-05-26
 */
#pragma once
#include "../../interfaces/sealer/SealerInterface.h"
#include <atomic>
#include <memory>

using namespace bcos;
using namespace bcos::sealer;

namespace bcos
{
namespace test
{
class FakeSealer : public SealerInterface
{
public:
    using Ptr = std::shared_ptr<FakeSealer>;
    FakeSealer() = default;
    ~FakeSealer() override {}

    void start() override {}
    void stop() override {}

    void asyncNoteUnSealedTxsSize(
        size_t _unSealedTxsSize, std::function<void(Error::Ptr)> _onRecvResponse) override
    {
        m_unSealedTxsSize = _unSealedTxsSize;
        _onRecvResponse(nullptr);
    }

    size_t unSealedTxsSize() const { return m_unSealedTxsSize; }

private:
    std::atomic<size_t> m_unSealedTxsSize = {0};
};
}  // namespace test
}  // namespace bcos