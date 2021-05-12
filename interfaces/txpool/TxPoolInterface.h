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
 * @file TxPoolInterface.h
 * @author: yujiechen
 * @date: 2021-04-07
 */
#pragma once
#include "../../interfaces/protocol/Block.h"
#include "../../interfaces/protocol/Transaction.h"
#include "../../interfaces/protocol/TransactionSubmitResult.h"
#include "../../libutilities/Error.h"
#include "TxPoolTypeDef.h"
namespace bcos
{
namespace txpool
{
class TxPoolInterface
{
public:
    using Ptr = std::shared_ptr<TxPoolInterface>;

    TxPoolInterface() = default;
    virtual ~TxPoolInterface() {}

    /**
     * @brief submit a transaction
     *
     * @param _tx the transaction to be submitted
     * @param _onChainCallback trigger this callback when receive the notification of transaction
     * on-chain
     */
    virtual void asyncSubmit(
        bytesPointer _tx, bcos::protocol::TxSubmitCallback _txSubmitCallback) = 0;

    /**
     * @brief fetch transactions from the txpool
     *
     * @param _txsLimit the max number of the transactions to be fetch
     * @param _avoidTxs list of transactions that need to be filtered
     * @param _sealCallback after the  txpool responds to the sealed txs, the callback is triggered
     */
    virtual void asyncSealTxs(size_t _txsLimit, TxsHashSetPtr _avoidTxs,
        std::function<void(Error::Ptr, bcos::protocol::ConstTransactionsPtr)> _sealCallback) = 0;

    /**
     * @brief verify transactions in Block for the consensus module
     *
     * @param _generatedNodeID the NodeID of the leader(When missing transactions, need to obtain
     * the missing transactions from Leader)
     * @param _blocks the block to be verified
     * @param _onVerifyFinished callback to be called after the block verification is over
     */
    virtual void asyncVerifyBlock(bcos::crypto::PublicPtr _generatedNodeID,
        bytesConstRef const& _block, std::function<void(Error::Ptr, bool)> _onVerifyFinished) = 0;


    /**
     * @brief Persistent transaction list
     *
     * @param _proposalHash the hash of the proposal that the txs belong to
     * @param _txsToStore the hash of the txs to be stored
     * @param _onTxsStored callback to be called after the given txs have been stored
     */
    virtual void asyncStoreTxs(bcos::crypto::HashType const& _proposalHash,
        bytesConstRef _txsToStore, std::function<void(Error::Ptr)> _onTxsStored) = 0;

    /**
     * @brief The dispatcher obtains the transaction list corresponding to the block from the
     * transaction pool
     *
     * @param _block the block to be filled with transactions
     * @param _onBlockFilled callback to be called after the block has been filled
     */
    virtual void asyncFillBlock(
        bcos::protocol::Block::Ptr _block, std::function<void(Error)> _onBlockFilled) = 0;

    /**
     * @brief After the blockchain is on-chain, the interface is called to notify the transaction
     * receipt and other information
     *
     * @param _onChainBlock Including transaction receipt, on-chain transaction hash list, block
     * header information
     * @param _onChainCallback
     */
    virtual void asyncNotifyBlockResult(bcos::protocol::BlockNumber _blockNumber,
        bcos::protocol::TransactionSubmitResultsPtr _txsResult,
        std::function<void(Error::Ptr)> _onNotifyFinished) = 0;

    /**
     * @brief the sync module calls this interface to obtain the new txs from the SDK
     *
     * @param _onReceiveNewTxs the callback to be called when receive new transaction list
     */
    virtual void asyncFetchNewTxs(size_t _txsLimit,
        std::function<void(Error::Ptr, bcos::protocol::ConstTransactionsPtr)> _onReceiveNewTxs) = 0;

    virtual void sendTxsSyncMessage(bcos::Error::Ptr _error, bcos::crypto::NodeIDPtr _nodeID,
        bytesPointer _data, std::function<void(bytesConstRef _respData)> _sendResponse);
};
}  // namespace txpool
}  // namespace bcos
