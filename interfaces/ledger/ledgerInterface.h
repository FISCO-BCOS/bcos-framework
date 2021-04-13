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
 * @brief interface for Ledger
 * @file Ledger.h
 * @author: kyonRay
 * @date: 2021-04-07
 */

#pragma once
#include <bcos-framework/interfaces/crypto/CommonType.h>
#include <bcos-framework/interfaces/protocol/Transaction.h>
#include <bcos-framework/interfaces/protocol/TransactionReceipt.h>
#include <bcos-framework/interfaces/protocol/Block.h>
#include <bcos-framework/interfaces/protocol/BlockHeader.h>
#include <bcos-framework/libutilities/Error.h>

namespace bcos
{
namespace ledger
{ 
class LedgerInterface
{
public:
    using MerkleProof = std::vector<std::pair<std::vector<std::string>, std::vector<std::string>>>;
    using MerkleProofPtr = std::shared_ptr<const MerkleProof>;
    LedgerInterface() = default;
    virtual ~LedgerInterface() {}

    /**
     * @brief commit a block
     *
     * @param _blockNumber the number of block to commit
     * @param _onCommitBlock trigger this callback when commit block in storage
     */
    virtual void asyncCommitBlock(bcos::protocol::BlockNumber _blockNumber,
        std::function<void(Error::Ptr)> _onCommitBlock) = 0;

    //    virtual bool checkAndBuildGenesisBlock(const std::shared_ptr<>) = 0;

    /**
     * @brief async get a transaction by hash
     *
     * @param _txHash hash of transaction
     * @param _onGetTx
     */
    virtual void asyncGetTxByHash(h256 const& _txHash,
        std::function<void(Error::Ptr, bcos::protocol::Transaction::ConstPtr)> _onGetTx) = 0;

    /**
     * @brief async get a transaction receipt by hash
     *
     * @param _txHash hash of transaction
     * @param _onGetTx
     */
    virtual void asyncGetTransactionReceiptByHash(
        h256 const& _txHash, std::function<void(Error::Ptr, bcos::protocol::TransactionReceipt::ConstPtr)> _onGetTx) = 0;

    /**
     * @brief async get total transaction count and latest block number
     * @param _callback callback a pair<txCount, latestBlockNumber>
     */
    virtual void asyncGetTotalTransactionCount(
        std::function<void(Error::Ptr, std::shared_ptr<std::pair<int64_t, bcos::protocol::BlockNumber>>)> _callback) = 0;

    /**
     * @brief async get total failed transaction count and latest block number
     * @param _callback callback a pair<failedTxCount, latestBlockNumber>
     */
    virtual void asyncGetTotalFailedTransactionCount(
        std::function<void(Error::Ptr, std::shared_ptr<const std::pair<int64_t, bcos::protocol::BlockNumber>>)> _callback) = 0;

    /**
     * @brief async get transaction receipt merkle proof in block
     * @param _block block ptr
     * @param _index transaction index in block
     * @param _onGetProof
     */
     virtual void asyncGetTransactionReceiptProof(const bcos::protocol::Block::Ptr _block, uint64_t const& _index,
                                             std::function<void(Error::Ptr, MerkleProofPtr)> _onGetProof) = 0;

     /**
      * @brief async get transaction merkle proof in block
      * @param _block block ptr
      * @param _index transaction index in block
      * @param _onGetProof
      */
     virtual void getTransactionProof(const bcos::protocol::Block::Ptr _block, uint64_t const& _index,
                                      std::function<void(Error::Ptr, MerkleProofPtr)> _onGetProof) = 0;

     /**
      * @brief async get transaction proof by tx hash
      * @param _txHash hash of transaction to get
      * @param _onGetProof
      */
     virtual void asyncGetTransactionProofByHash(h256 const& _txHash,
                                            std::function<void(Error::Ptr, MerkleProofPtr)> _onGetProof) = 0;


    /**
     * @brief async get latest block number
     *
     * @param _onGetBlock
     */
    virtual void asyncGetBlockNumber(std::function<void(Error::Ptr, bcos::protocol::BlockNumber)> _onGetBlock) = 0;

    /**
     * @brief async get block hash by block number
     *
     * @param _onGetBlock
     */
    virtual void asyncGetBlockHashByNumber(bcos::protocol::BlockNumber,
        std::function<void(Error::Ptr, std::shared_ptr<const h256>)> _onGetBlock) = 0;

    /**
     * @brief async get a block by hash
     *
     * @param _blockHash hash of block to get
     * @param _onGetBlock
     */
    virtual void asyncGetBlockByHash(h256 const& _blockHash,
        std::function<void(Error::Ptr, bcos::protocol::Block::Ptr)> _onGetBlock) = 0;

    /**
     * @brief async get block by blockNumber
     *
     * @param _blockNumber number of block
     * @param _onGetBlock
     */
    virtual void asyncGetBlockByNumber(bcos::protocol::BlockNumber _blockNumber,
        std::function<void(Error::Ptr, bcos::protocol::Block::Ptr)> _onGetBlock) = 0;

    /**
     * @brief async get a encoded block by number
     * @param _blockNumber number of block
     * @param _onGetBlock
     */
    virtual void asyncGetBlockEncodedByNumber(bcos::protocol::BlockNumber _blockNumber,
                                              std::function<void(Error::Ptr, std::shared_ptr<const bytes>)> _onGetBlock) = 0;

    /**
     * @brief async get a block header by number
     * @param _blockNumber number of block
     * @param _onGetBlock callback when get a block, (error, pair<blockHeader, signatureList>)
     */
    virtual void asyncGetBlockHeaderByNumber(bcos::protocol::BlockNumber _blockNumber,
        std::function<void(Error::Ptr, std::shared_ptr<const std::pair<bcos::protocol::BlockHeader::Ptr, bcos::protocol::SignatureListPtr>>)> _onGetBlock) = 0;

    /**
     * @brief async get block header by block hash
     * @param _blockHash hash of block
     * @param _onGetBlock callback when get a block, (error, pair<blockHeader, signatureList>)
     */
    virtual void asyncGetBlockHeaderByHash(h256 const& _blockHash,
        std::function<void(Error::Ptr, std::shared_ptr<const std::pair<bcos::protocol::BlockHeader::Ptr, bcos::protocol::SignatureListPtr>>)> _onGetBlock) = 0;

    /**
     * @brief async get contract code by tableID and codeName
     * @param _tableID table to get
     * @param _codeAddress contract code name
     * @param _onGetCode
     */
    virtual void asyncGetCode(std::string const& _tableID, bcos::Address _codeAddress,
        std::function<void(Error::Ptr, std::shared_ptr<const bytes>)> _onGetCode) = 0;

    /**
     * @brief async get system config by table key
     * @param _key
     * @param _onGetConfig callback when get config, <value, latest block number>
     */
    virtual void asyncGetSystemConfigByKey(std::string const& _key,
        std::function<void(Error::Ptr, std::shared_ptr<const std::pair<std::string, bcos::protocol::BlockNumber>>)> _onGetConfig) = 0;

    /**
     * @brief async get nonce list in specific block
     * @param _blockNumber number of block to get
     * @param _onGetList
     */
    virtual void asyncGetNonceList(bcos::protocol::BlockNumber _blockNumber,
                           std::function<void(Error::Ptr, bcos::protocol::NonceListPtr)> _onGetList) = 0;
};
} // namespace ledger
} // namespace bcos
