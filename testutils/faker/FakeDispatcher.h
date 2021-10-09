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
 * @brief faker for the Dispatcher
 * @file FakeDispatcher.h
 * @author: yujiechen
 * @date 2021-05-28

 */
#pragma once
#include "../../interfaces/crypto/CryptoSuite.h"
#include "../../interfaces/dispatcher/DispatcherInterface.h"
#include "../../interfaces/ledger/LedgerInterface.h"
#include "../../interfaces/protocol/BlockFactory.h"
#include "../../interfaces/storage/StorageInterface.h"
#include "../../interfaces/txpool/TxPoolInterface.h"
#include "../../libstorage/StateStorage.h"
#include "../protocol/FakeTransactionReceipt.h"
using namespace bcos;
using namespace bcos::dispatcher;
using namespace bcos::protocol;
using namespace bcos::txpool;
using namespace bcos::ledger;
using namespace bcos::storage;
using namespace bcos::crypto;

namespace bcos
{
namespace test
{
class FakeDispatcher : public DispatcherInterface
{
public:
    using Ptr = std::shared_ptr<FakeDispatcher>;
    FakeDispatcher() = default;
    FakeDispatcher(LedgerInterface::Ptr _ledger, StorageInterface::Ptr _storage,
        CryptoSuite::Ptr _cryptoSuite, BlockFactory::Ptr _blockFactory)
      : m_ledger(_ledger),
        m_storage(_storage),
        m_cryptoSuite(_cryptoSuite),
        m_blockFactory(_blockFactory)
    {}
    ~FakeDispatcher() override {}

    void setTxPool(TxPoolInterface::Ptr _txpool) { m_txpool = _txpool; }

    void asyncExecuteBlock(const Block::Ptr& _block, bool _verify,
        std::function<void(const Error::Ptr&, const BlockHeader::Ptr&)> _callback, ssize_t) override
    {
        auto blockHeader = _block->blockHeader();
        if (m_blockFactory)
        {
            blockHeader =
                m_blockFactory->blockHeaderFactory()->populateBlockHeader(_block->blockHeader());
        }
        if (!m_txpool)
        {
            _callback(nullptr, blockHeader);
            return;
        }
        if (!_verify)
        {
            fillBlockAndPrecommit(_block, _callback, blockHeader);
        }
        else
        {
            preCommitBlock(_block, blockHeader, _callback);
        }
    }

    void preCommitBlock(const Block::Ptr& _block, BlockHeader::Ptr _header,
        std::function<void(const Error::Ptr&, const BlockHeader::Ptr&)> _callback)
    {
        auto tableFactory = std::make_shared<StateStorage>(m_storage);
        for (size_t i = 0; i < _block->transactionsHashSize(); i++)
        {
            _block->appendReceipt(testPBTransactionReceipt(m_cryptoSuite));
        }

        // TODO: remove this method
        (void)_header;
        (void)_callback;

        /*
        m_ledger->asyncStoreReceipts(tableFactory, _block, [](Error::Ptr) {});
        auto txsRoot = _block->calculateTransactionRoot(false);
        _header->setTxsRoot(txsRoot);
        _callback(nullptr, _header);
        */
    }

    void fillBlockAndPrecommit(const Block::Ptr& _block,
        std::function<void(const Error::Ptr&, const BlockHeader::Ptr&)> _callback,
        BlockHeader::Ptr _header)
    {
        auto txsHashList = std::make_shared<HashList>();
        for (size_t i = 0; i < _block->transactionsHashSize(); i++)
        {
            txsHashList->push_back(_block->transactionHash(i));
        }
        m_txpool->asyncFillBlock(txsHashList,
            [this, _block, _callback, _header](Error::Ptr _error, TransactionsPtr _txs) {
                if (_error)
                {
                    _callback(_error, _header);
                    return;
                }
                for (auto tx : *_txs)
                {
                    _block->appendTransaction(tx);
                }
                preCommitBlock(_block, _header, _callback);
            });
    }

    // useless for PBFT, maybe useful for executors
    void asyncGetLatestBlock(std::function<void(const Error::Ptr&, const Block::Ptr&)>) override {}

    // useless for PBFT, maybe useful for executors
    void asyncNotifyExecutionResult(const Error::Ptr&, bcos::crypto::HashType const&,
        const std::shared_ptr<BlockHeader>&, std::function<void(const Error::Ptr&)>) override
    {}

    void stop() override {}
    void start() override {}

private:
    TxPoolInterface::Ptr m_txpool;
    LedgerInterface::Ptr m_ledger;
    StorageInterface::Ptr m_storage;
    CryptoSuite::Ptr m_cryptoSuite;
    BlockFactory::Ptr m_blockFactory;
};
}  // namespace test
}  // namespace bcos