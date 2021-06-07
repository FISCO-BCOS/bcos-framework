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
 * @file SealerFactory.h
 * @author: yujiechen
 * @date: 2021-05-20
 */
#pragma once

#include "../interfaces/sealer/SealerInterface.h"
#include "SealerConfig.h"
namespace bcos
{
namespace sealer
{
class SealerFactory
{
public:
    using Ptr = std::shared_ptr<SealerFactory>;
    SealerFactory(bcos::protocol::BlockFactory::Ptr _blockFactory,
        bcos::txpool::TxPoolInterface::Ptr _txpool, unsigned _minSealTime);
    virtual ~SealerFactory() {}

    virtual void init(bcos::consensus::ConsensusInterface::Ptr _consensus);

    virtual SealerInterface::Ptr sealer() { return m_sealer; }

protected:
    SealerInterface::Ptr m_sealer;
    SealerConfig::Ptr m_sealerConfig;
};
}  // namespace sealer
}  // namespace bcos