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
#include "Common.h"
#include "interfaces/protocol/CommonError.h"

using namespace bcos;
using namespace bcos::sealer;
using namespace bcos::crypto;
using namespace bcos::protocol;

void SealingManager::appendTransactions(HashListPtr _fetchedTxs)
{
    SEAL_LOG(DEBUG) << LOG_DESC("appendTransactions") << LOG_KV("size", _fetchedTxs->size());
    WriteGuard l(x_pendingTxs);
    m_pendingTxs->insert(m_pendingTxs->end(), _fetchedTxs->begin(), _fetchedTxs->end());
    m_onReady();
}

bool SealingManager::shouldGenerateProposal()
{
    if (m_sealingNumber < m_startSealingNumber || m_sealingNumber > m_endSealingNumber)
    {
        clearPendingTxs();
        return false;
    }
    ReadGuard l(x_pendingTxs);
    if (m_pendingTxs->size() >= m_maxTxsPerBlock || reachMinSealTimeCondition())
    {
        return true;
    }
    return false;
}

void SealingManager::clearPendingTxs()
{
    UpgradableGuard l(x_pendingTxs);
    if (m_pendingTxs->size() == 0)
    {
        return;
    }
    // return the txs back to the txpool
    SEAL_LOG(INFO) << LOG_DESC("clearPendingTxs: return back the unhandled transactions")
                   << LOG_KV("size", m_pendingTxs->size());
    HashListPtr unHandledTxs =
        std::make_shared<HashList>(m_pendingTxs->begin(), m_pendingTxs->end());
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
}

void SealingManager::notifyResetTxsFlag(HashListPtr _txsHashList, bool _flag)
{
    m_config->txpool()->asyncMarkTxs(
        _txsHashList, _flag, [this, _txsHashList, _flag](Error::Ptr _error) {
            if (_error->errorCode() == CommonError::SUCCESS)
            {
                SEAL_LOG(DEBUG) << LOG_DESC("asyncMarkTxs success");
                return;
            }
            SEAL_LOG(DEBUG) << LOG_DESC("asyncMarkTxs failed, retry now");
            this->notifyResetTxsFlag(_txsHashList, _flag);
        });
}

Block::Ptr SealingManager::generateProposal()
{
    if (!shouldGenerateProposal())
    {
        return nullptr;
    }
    WriteGuard l(x_pendingTxs);
    auto block = m_config->blockFactory()->createBlock();
    auto blockHeader = m_config->blockFactory()->blockHeaderFactory()->createBlockHeader();
    blockHeader->setNumber(m_sealingNumber);
    blockHeader->setTimestamp(utcTime());
    block->setBlockHeader(blockHeader);

    for (size_t i = 0; i <= std::min((size_t)m_maxTxsPerBlock, m_pendingTxs->size()); i++)
    {
        block->setTransactionHash(i, m_pendingTxs->front());
        m_pendingTxs->pop_front();
    }
    m_sealingNumber++;
    m_lastSealTime = utcSteadyTime();
    return block;
}

bool SealingManager::reachMinSealTimeCondition()
{
    ReadGuard l(x_pendingTxs);
    if (m_pendingTxs->size() == 0)
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
    if (m_fetchingTxs || m_unsealedTxsSize == 0)
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
    ReadGuard l(x_pendingTxs);
    if (txsSizeToFetch <= m_pendingTxs->size())
    {
        return 0;
    }
    return (txsSizeToFetch - m_pendingTxs->size());
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
        SEAL_LOG(DEBUG) << LOG_DESC("No need to fetchTransactions");
        return;
    }
    // try to fetch transactions
    m_fetchingTxs = true;
    auto self = std::weak_ptr<SealingManager>(shared_from_this());
    m_config->txpool()->asyncSealTxs(
        txsToFetch, nullptr, [self](Error::Ptr _error, bcos::crypto::HashListPtr _txsHashList) {
            try
            {
                auto sealingMgr = self.lock();
                if (!sealingMgr)
                {
                    return;
                }
                if (_error->errorCode() != CommonError::SUCCESS)
                {
                    SEAL_LOG(WARNING) << LOG_DESC("fetchTransactions exception")
                                      << LOG_KV("returnCode", _error->errorCode())
                                      << LOG_KV("returnMsg", _error->errorMessage());
                    sealingMgr->m_fetchingTxs = false;
                    return;
                }
                sealingMgr->appendTransactions(_txsHashList);
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