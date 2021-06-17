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
 * @file SealingManager.h
 * @author: yujiechen
 * @date: 2021-05-14
 */
#pragma once
#include "../interfaces/protocol/BlockFactory.h"
#include "../libutilities/CallbackCollectionHandler.h"
#include "../libutilities/ThreadPool.h"
#include "SealerConfig.h"
namespace bcos
{
namespace sealer
{
using TxsHashQueue = std::deque<bcos::crypto::HashType>;
class SealingManager : public std::enable_shared_from_this<SealingManager>
{
public:
    using Ptr = std::shared_ptr<SealingManager>;
    using ConstPtr = std::shared_ptr<SealingManager const>;
    explicit SealingManager(SealerConfig::Ptr _config)
      : m_config(_config),
        m_pendingTxs(std::make_shared<TxsHashQueue>()),
        m_worker(std::make_shared<ThreadPool>("sealerWorker", 1))
    {}

    virtual ~SealingManager() {}

    virtual bool shouldGenerateProposal();
    virtual bool shouldFetchTransaction();

    bcos::protocol::Block::Ptr generateProposal();
    virtual void setUnsealedTxsSize(size_t _unsealedTxsSize)
    {
        m_unsealedTxsSize = _unsealedTxsSize;
    }

    virtual void resetSealingInfo(
        ssize_t _startSealingNumber, ssize_t _endSealingNumber, size_t _maxTxsPerBlock)
    {
        m_startSealingNumber = _startSealingNumber;
        m_endSealingNumber = _endSealingNumber;
        m_maxTxsPerBlock = _maxTxsPerBlock;
        m_sealingNumber = _startSealingNumber;
        m_lastSealTime = utcSteadyTime();
    }
    virtual void fetchTransactions();

    template <class T>
    bcos::Handler<> onReady(T const& _t)
    {
        return m_onReady.add(_t);
    }

    virtual void appendTransactions(bcos::crypto::HashListPtr _fetchedTxs);

protected:
    virtual bool reachMinSealTimeCondition();
    virtual void clearPendingTxs();
    virtual void notifyResetTxsFlag(
        bcos::crypto::HashListPtr _txsHash, bool _flag, size_t _retryTime = 0);

    virtual int64_t txsSizeExpectedToFetch();

private:
    SealerConfig::Ptr m_config;
    std::shared_ptr<TxsHashQueue> m_pendingTxs;
    SharedMutex x_pendingTxs;

    ThreadPool::Ptr m_worker;

    std::atomic<uint64_t> m_lastSealTime = {0};

    // the invalid sealingNumber is -1
    std::atomic<ssize_t> m_sealingNumber = {-1};
    std::atomic<size_t> m_unsealedTxsSize = {0};

    std::atomic<ssize_t> m_startSealingNumber = {0};
    std::atomic<ssize_t> m_endSealingNumber = {0};
    std::atomic<size_t> m_maxTxsPerBlock = {0};


    bcos::CallbackCollectionHandler<> m_onReady;

    std::atomic_bool m_fetchingTxs = {false};
};
}  // namespace sealer
}  // namespace bcos