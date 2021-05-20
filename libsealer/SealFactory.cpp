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
 * @file SealerFactory.cpp
 * @author: yujiechen
 * @date: 2021-05-20
 */
#include "Sealer.h"
#include "SealerFactory.h"
using namespace bcos;
using namespace bcos::sealer;

SealerFactory::SealerFactory(bcos::protocol::BlockFactory::Ptr _blockFactory,
    bcos::txpool::TxPoolInterface::Ptr _txpool, unsigned _minSealTime)
{
    m_sealerConfig = std::make_shared<SealerConfig>(_blockFactory, _txpool);
    m_sealerConfig->setMinSealTime(_minSealTime);
    m_sealer = std::make_shared<Sealer>(m_sealerConfig);
}

void SealerFactory::init(bcos::consensus::ConsensusInterface::Ptr _consensus)
{
    m_sealerConfig->setConsensusInterface(_consensus);
}