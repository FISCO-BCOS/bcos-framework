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
 * @file SealingManager.cpp
 * @author: yujiechen
 * @date: 2021-05-14
 */
#include "SealingManager.h"

using namespace bcos;
using namespace bcos::sealer;
using namespace bcos::crypto;
using namespace bcos::protocol;

void SealingManager::resetSealing()
{
    SEAL_LOG(INFO) << LOG_DESC("resetSealing") << LOG_KV("startNum", m_startSealingNumber)
                   << LOG_KV("endNum", m_endSealingNumber) << LOG_KV("sealingNum", m_sealingNumber)
                   << LOG_KV("pendingTxs", pendingTxsSize());
    m_sealingNumber = m_endSealingNumber + 1;
    clearPendingTxs();
}

void SealingManager::appendTransactions(HashListPtr _fetchedTxs, bool _systemTx)
{
    WriteGuard l(x_pendingTxs);
    // append the system transactions
    if (_systemTx)
    {
        m_pendingSysTxs->insert(m_pendingSysTxs->end(), _fetchedTxs->begin(), _fetchedTxs->end());
    }
    else
    {
        // append the common transactions
        m_pendingTxs->insert(m_pendingTxs->end(), _fetchedTxs->begin(), _fetchedTxs->end());
    }
    m_onReady();
}

bool SealingManager::shouldGenerateProposal()
{
    if (m_sealingNumber < m_startSealingNumber || m_sealingNumber > m_endSealingNumber)
    {
        clearPendingTxs();
        return false;
    }
    // should wait the given block submit to the ledger
    if (m_currentNumber < m_waitUntil)
    {
        return false;
    }
    // check the txs size
    auto txsSize = pendingTxsSize();
    if (txsSize >= m_maxTxsPerBlock || reachMinSealTimeCondition())
    {
        return true;
    }
    return false;
}

void SealingManager::clearPendingTxs()
{
    UpgradableGuard l(x_pendingTxs);
    auto pendingTxsSize = m_pendingTxs->size() + m_pendingSysTxs->size();
    if (pendingTxsSize == 0)
    {
        return;
    }
    // return the txs back to the txpool
    SEAL_LOG(INFO) << LOG_DESC("clearPendingTxs: return back the unhandled transactions")
                   << LOG_KV("size", pendingTxsSize);
    HashListPtr unHandledTxs =
        std::make_shared<HashList>(m_pendingTxs->begin(), m_pendingTxs->end());
    unHandledTxs->insert(unHandledTxs->end(), m_pendingSysTxs->begin(), m_pendingSysTxs->end());
    auto self = std::weak_ptr<SealingManager>(shared_from_this());
    m_worker->enqueue([self, unHandledTxs]() {
        try
        {
            auto sealerMgr = self.lock();
            if (!sealerMgr)
            {
                return;
            }
            sealerMgr->notifyResetTxsFlag(unHandledTxs, false);
        }
        catch (std::exception const& e)
        {
            SEAL_LOG(WARNING)
                << LOG_DESC("shouldGenerateProposal: return back the unhandled txs exception")
                << LOG_KV("error", boost::diagnostic_information(e));
        }
    });
    UpgradeGuard ul(l);
    m_pendingTxs->clear();
    m_pendingSysTxs->clear();
}

void SealingManager::notifyResetTxsFlag(HashListPtr _txsHashList, bool _flag, size_t _retryTime)
{
    m_config->txpool()->asyncMarkTxs(_txsHashList, _flag, 0, bcos::crypto::HashType(),
        [this, _txsHashList, _flag, _retryTime](Error::Ptr _error) {
            if (_error == nullptr)
            {
                return;
            }
            SEAL_LOG(WARNING) << LOG_DESC("asyncMarkTxs failed, retry now");
            if (_retryTime >= 3)
            {
                return;
            }
            this->notifyResetTxsFlag(_txsHashList, _flag, _retryTime + 1);
        });
}

void SealingManager::notifyResetProposal(bcos::protocol::Block::Ptr _block)
{
    auto txsHashList = std::make_shared<HashList>();
    for (size_t i = 0; i < _block->transactionsHashSize(); i++)
    {
        txsHashList->push_back(_block->transactionHash(i));
    }
    notifyResetTxsFlag(txsHashList, false);
}
std::pair<bool, bcos::protocol::Block::Ptr> SealingManager::generateProposal()
{
    if (!shouldGenerateProposal())
    {
        return std::pair(false, nullptr);
    }
    WriteGuard l(x_pendingTxs);
    m_sealingNumber = std::max(m_sealingNumber.load(), m_currentNumber.load() + 1);
    auto block = m_config->blockFactory()->createBlock();
    auto blockHeader = m_config->blockFactory()->blockHeaderFactory()->createBlockHeader();
    blockHeader->setNumber(m_sealingNumber);
    blockHeader->setTimestamp(utcTime());
    block->setBlockHeader(blockHeader);
    auto txsSize =
        std::min((size_t)m_maxTxsPerBlock, (m_pendingTxs->size() + m_pendingSysTxs->size()));
    // prioritize seal from the system txs list
    auto systemTxsSize = std::min(txsSize, m_pendingSysTxs->size());
    if (m_pendingSysTxs->size() > 0)
    {
        m_waitUntil.store(m_sealingNumber);
        SEAL_LOG(INFO) << LOG_DESC("seal the system transactions")
                       << LOG_KV("sealNextBlockUntil", m_waitUntil)
                       << LOG_KV("curNum", m_currentNumber);
    }
    bool containSysTxs = false;
    for (size_t i = 0; i < systemTxsSize; i++)
    {
        block->appendTransactionHash(m_pendingSysTxs->front());
        m_pendingSysTxs->pop_front();
        containSysTxs = true;
    }
    for (size_t i = systemTxsSize; i < txsSize; i++)
    {
        block->appendTransactionHash(m_pendingTxs->front());
        m_pendingTxs->pop_front();
    }
    m_sealingNumber++;

    m_lastSealTime = utcSteadyTime();
    // Note: When the last block(N) sealed by this node contains system transactions,
    //       if other nodes do not wait until block(N) is committed and directly seal block(N+1),
    //       will cause system exceptions.
    return std::pair(containSysTxs, block);
}

size_t SealingManager::pendingTxsSize()
{
    ReadGuard l(x_pendingTxs);
    return m_pendingSysTxs->size() + m_pendingTxs->size();
}
bool SealingManager::reachMinSealTimeCondition()
{
    auto txsSize = pendingTxsSize();
    if (txsSize == 0)
    {
        return false;
    }
    if ((utcSteadyTime() - m_lastSealTime) < m_config->minSealTime())
    {
        return false;
    }
    return true;
}

bool SealingManager::shouldFetchTransaction()
{
    // fetching transactions currently
    if (m_fetchingTxs.load() || m_unsealedTxsSize == 0)
    {
        return false;
    }
    // no need to sealing
    if (m_sealingNumber < m_startSealingNumber || m_sealingNumber > m_endSealingNumber)
    {
        return false;
    }
    return true;
}

int64_t SealingManager::txsSizeExpectedToFetch()
{
    auto txsSizeToFetch = (m_endSealingNumber - m_sealingNumber + 1) * m_maxTxsPerBlock;
    auto txsSize = pendingTxsSize();
    if (txsSizeToFetch <= txsSize)
    {
        return 0;
    }
    return (txsSizeToFetch - txsSize);
}

void SealingManager::fetchTransactions()
{
    if (!shouldFetchTransaction())
    {
        return;
    }
    auto txsToFetch = txsSizeExpectedToFetch();
    if (txsToFetch == 0)
    {
        return;
    }
    // try to fetch transactions
    m_fetchingTxs = true;
    auto self = std::weak_ptr<SealingManager>(shared_from_this());
    m_config->txpool()->asyncSealTxs(txsToFetch, nullptr,
        [self](Error::Ptr _error, bcos::crypto::HashListPtr _txsHashList,
            bcos::crypto::HashListPtr _sysTxsList) {
            try
            {
                auto sealingMgr = self.lock();
                if (!sealingMgr)
                {
                    return;
                }
                if (_error != nullptr)
                {
                    SEAL_LOG(WARNING) << LOG_DESC("fetchTransactions exception")
                                      << LOG_KV("returnCode", _error->errorCode())
                                      << LOG_KV("returnMsg", _error->errorMessage());
                    sealingMgr->m_fetchingTxs = false;
                    return;
                }
                sealingMgr->appendTransactions(_txsHashList, false);
                sealingMgr->appendTransactions(_sysTxsList, true);
                sealingMgr->m_fetchingTxs = false;
            }
            catch (std::exception const& e)
            {
                SEAL_LOG(WARNING) << LOG_DESC("fetchTransactions: onRecv sealed txs failed")
                                  << LOG_KV("error", boost::diagnostic_information(e))
                                  << LOG_KV("fetchedTxsSize", _txsHashList->size())
                                  << LOG_KV("returnCode", _error->errorCode())
                                  << LOG_KV("returnMsg", _error->errorMessage());
            }
        });
}