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
 * @brief interface for Ledger
 * @file LedgerInterface.h
 * @author: kyonRay
 * @date: 2021-04-07
 */

#pragma once

#include "../../interfaces/crypto/CommonType.h"
#include "../../interfaces/protocol/Block.h"
#include "../../interfaces/protocol/BlockHeader.h"
#include "../../interfaces/protocol/Transaction.h"
#include "../../interfaces/protocol/TransactionReceipt.h"
#include "../../libutilities/Error.h"
#include "LedgerConfig.h"
#include "LedgerTypeDef.h"
#include <gsl/span>
#include <map>


namespace bcos
{
namespace ledger
{
class LedgerInterface
{
public:
    using Ptr = std::shared_ptr<LedgerInterface>;
    LedgerInterface() = default;
    virtual ~LedgerInterface() {}

    /**
     * @brief async commit a block
     *
     * @param _blockNumber the number of block to commit, txs had been stored in asyncPreStoreTxs()
     * @param _signList the signature list of block header to commit,
     *                  if _signList.empty(), it means sync module call this interface or error
     * happened if not, it means consensus call this
     * @param _onCommitBlock trigger this callback when commit block in storage
     */
    virtual void asyncCommitBlock(bcos::protocol::BlockNumber _blockNumber,
        const gsl::span<const protocol::Signature>& _signList,
        std::function<void(Error::Ptr, LedgerConfig::Ptr)> _onCommitBlock) = 0;

    /**
     * @brief async pre-store tx in block when tx pool verify
     * @param _txToStore
     * @param _number pre-store block number
     * @param _onTxsStored callback
     */
    virtual void asyncPreStoreTransaction(bytesPointer _txToStore,
        const crypto::HashType& _txHash, std::function<void(Error::Ptr)> _onTxStored) = 0;

    /**
     * @brief async get block by blockNumber
     * @param _blockNumber number of block
     * @param _blockFlag flag bit of what the block be callback contains,
     *                   you can checkout all flags in LedgerTypeDef.h
     * @param _onGetBlock
     *
     * @example
     * asyncGetBlockDataByNumber(10, HEADER|TRANSACTIONS, [](error, block){ doSomething(); });
     */
    virtual void asyncGetBlockDataByNumber(protocol::BlockNumber _blockNumber, int32_t _blockFlag,
        std::function<void(Error::Ptr, protocol::Block::Ptr)> _onGetBlock) = 0;

    /**
     * @brief async get latest block number
     *
     * @param _onGetBlock
     */
    virtual void asyncGetBlockNumber(
        std::function<void(Error::Ptr, protocol::BlockNumber)> _onGetBlock) = 0;

    /**
     * @brief async get block hash by block number
     * @param _blockNumber the number of block to get
     * @param _onGetBlock
     */
    virtual void asyncGetBlockHashByNumber(protocol::BlockNumber _blockNumber,
        std::function<void(Error::Ptr, crypto::HashType const&)> _onGetBlock) = 0;

    /**
     * @brief async get block number by block hash
     * @param _blockHash the hash of block to get
     * @param _onGetBlock
     */
    virtual void asyncGetBlockNumberByHash(crypto::HashType const& _blockHash,
        std::function<void(Error::Ptr, protocol::BlockNumber)> _onGetBlock) = 0;

    /**
     * @brief async get a transaction by transaction hash
     * @param _txHash hash of transaction
     * @param _withProof if true then it will callback MerkleProofPtr in _onGetTx
     *                   if false then MerkleProofPtr will be nullptr
     * @param _onGetTx
     */
    virtual void asyncGetTransactionByHash(crypto::HashType const& _txHash, bool _withProof,
        std::function<void(Error::Ptr, protocol::Transaction::ConstPtr, MerkleProofPtr)>
            _onGetTx) = 0;

    /**
     * @brief async get a transaction receipt by tx hash
     * @param _txHash hash of transaction
     * @param _withProof if true then it will callback MerkleProofPtr in _onGetTx
     *                   if false then MerkleProofPtr will be nullptr
     * @param _onGetTx
     */
    virtual void asyncGetTransactionReceiptByHash(crypto::HashType const& _txHash, bool _withProof,
        std::function<void(Error::Ptr, protocol::TransactionReceipt::ConstPtr, MerkleProofPtr)>
            _onGetTx) = 0;

    /**
     * @brief async get transaction by block number and index
     * @param _blockNumber number of block
     * @param _index index of tx in block txList
     * @param _withProof if true then it will callback MerkleProofPtr in _onGetTx
     *                   if false then MerkleProofPtr will be nullptr
     * @param _onGetTx
     */
    virtual void asyncGetTransactionByBlockNumberAndIndex(protocol::BlockNumber _blockNumber,
        int64_t _index, bool _withProof,
        std::function<void(Error::Ptr, protocol::Transaction::ConstPtr, MerkleProofPtr)>
            _onGetTx) = 0;

    /**
     * @brief async get a tx receipt by block number and index
     * @param _blockNumber number of block
     * @param _index index of tx receipt in block receipt list
     * @param _withProof if true then it will callback MerkleProofPtr in _onGetTx
     *                   if false then MerkleProofPtr will be nullptr
     * @param _onGetTx
     */
    virtual void asyncGetReceiptByBlockNumberAndIndex(protocol::BlockNumber _blockNumber,
        int64_t _index, bool _withProof,
        std::function<void(Error::Ptr, protocol::TransactionReceipt::ConstPtr, MerkleProofPtr)>
            _onGetTx) = 0;

    /**
     * @brief async get total transaction count and latest block number
     * @param _callback callback totalTxCount, totalFailedTxCount, and latest block number
     */
    virtual void asyncGetTotalTransactionCount(std::function<void(Error::Ptr, int64_t _totalTxCount,
            int64_t _failedTxCount, protocol::BlockNumber _latestBlockNumber)>
            _callback) = 0;

    /**
     * @brief async get system config by table key
     * @param _key the key of row, you can checkout all key in LedgerTypeDef.h
     * @param _onGetConfig callback when get config, <value, latest block number>
     */
    virtual void asyncGetSystemConfigByKey(std::string const& _key,
        std::function<void(Error::Ptr, std::string, protocol::BlockNumber)> _onGetConfig) = 0;

    /**
     * @brief async get node list by type, can be sealer or observer
     * @param _type the type of node, CONSENSUS_SEALER or CONSENSUS_OBSERVER
     * @param _onGetConfig
     */
    virtual void asyncGetNodeListByType(std::string const& _type,
        std::function<void(Error::Ptr, consensus::ConsensusNodeListPtr)> _onGetConfig) = 0;

    /**
     * @brief async get a batch of nonce lists in blocks
     * @param _startNumber start block number
     * @param _offset batch offset, if batch is 0, then callback nonce list in start block number;
     * if (_startNumber + _offset) > latest block number, then callback nonce lists in
     * [_startNumber, latest number]
     * @param _onGetList
     */
    virtual void asyncGetNonceList(protocol::BlockNumber _startNumber, int64_t _offset,
        std::function<void(
            Error::Ptr, std::shared_ptr<std::map<protocol::BlockNumber, protocol::NonceListPtr>>)>
            _onGetList) = 0;
};
}  // namespace ledger
}  // namespace bcos
