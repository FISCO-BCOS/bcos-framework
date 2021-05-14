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
 * @file Sealer.cpp
 * @author: yujiechen
 * @date: 2021-05-14
 */
#include "Sealer.h"
#include "Common.h"
#include "interfaces/protocol/CommonError.h"
using namespace bcos;
using namespace bcos::sealer;
using namespace bcos::protocol;

void Sealer::start()
{
    if (m_running)
    {
        SEAL_LOG(INFO) << LOG_DESC("the sealer has already been started");
        return;
    }
    SEAL_LOG(INFO) << LOG_DESC("start the sealer");
    startWorking();
    m_running = true;
}

void Sealer::stop()
{
    if (!m_running)
    {
        SEAL_LOG(INFO) << LOG_DESC("the sealer has already been stopped");
        return;
    }
    finishWorker();
    if (isWorking())
    {
        stopWorking();
        // will not restart worker, so terminate it
        terminate();
    }
}

void Sealer::asyncNotifySealProposal(size_t _proposalStartIndex, size_t _proposalEndIndex,
    size_t _maxTxsPerBlock, std::function<void(Error::Ptr)> _onRecvResponse)
{
    m_sealingManager->resetSealingInfo(_proposalStartIndex, _proposalEndIndex, _maxTxsPerBlock);
    _onRecvResponse(std::make_shared<Error>(CommonError::SUCCESS, "SUCCESS"));
    SEAL_LOG(INFO) << LOG_DESC("asyncNotifySealProposal")
                   << LOG_KV("startIndex", _proposalStartIndex)
                   << LOG_KV("endIndex", _proposalEndIndex)
                   << LOG_KV("maxTxsPerBlock", _maxTxsPerBlock);
}

void Sealer::asyncNoteUnSealedTxsSize(
    size_t _unsealedTxsSize, std::function<void(Error::Ptr)> _onRecvResponse)
{
    m_sealingManager->setUnsealedTxsSize(_unsealedTxsSize);
    _onRecvResponse(std::make_shared<Error>(CommonError::SUCCESS, "SUCCESS"));
    SEAL_LOG(INFO) << LOG_DESC("asyncNoteUnSealedTxsSize")
                   << LOG_KV("unsealedTxsSize", _unsealedTxsSize);
}

void Sealer::executeWorker()
{
    if (!m_sealingManager->shouldGenerateProposal() && !m_sealingManager->shouldFetchTransaction())
    {
        ///< 10 milliseconds to next loop
        boost::unique_lock<boost::mutex> l(x_signalled);
        m_signalled.wait_for(l, boost::chrono::milliseconds(1));
    }
    // try to generateProposal
    if (m_sealingManager->shouldGenerateProposal())
    {
        auto proposal = m_sealingManager->generateProposal();
        // TODO: submit the proposal to the consensus module
        SEAL_LOG(DEBUG) << LOG_DESC("Generate proposal")
                        << LOG_KV("number", proposal->blockHeader()->number())
                        << LOG_KV("hash", proposal->blockHeader()->hash().abridged())
                        << LOG_KV("txsSize", proposal->transactionsHashSize());
    }
    // try to fetch transactions
    if (m_sealingManager->shouldFetchTransaction())
    {
        m_sealingManager->fetchTransactions();
    }
}
