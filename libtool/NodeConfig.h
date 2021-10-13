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
 * @brief configuration for the node
 * @file NodeConfig.h
 * @author: yujiechen
 * @date 2021-06-10
 */
#pragma once
#include "../interfaces/consensus/ConsensusNodeInterface.h"
#include "../interfaces/crypto/KeyFactory.h"
#include "../interfaces/ledger/LedgerConfig.h"
#include "../libutilities/Log.h"
#include "Exceptions.h"
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#define NodeConfig_LOG(LEVEL) BCOS_LOG(LEVEL) << LOG_BADGE("NodeConfig")
namespace bcos
{
namespace tool
{
class NodeConfig
{
public:
    using Ptr = std::shared_ptr<NodeConfig>;
    NodeConfig() : m_ledgerConfig(std::make_shared<bcos::ledger::LedgerConfig>()) {}

    explicit NodeConfig(bcos::crypto::KeyFactory::Ptr _keyFactory);
    virtual ~NodeConfig() {}

    virtual void loadConfig(std::string const& _configPath)
    {
        boost::property_tree::ptree iniConfig;
        boost::property_tree::read_ini(_configPath, iniConfig);
        loadConfig(iniConfig);
    }

    virtual void loadGenesisConfig(std::string const& _genesisConfigPath)
    {
        boost::property_tree::ptree genesisConfig;
        boost::property_tree::read_ini(_genesisConfigPath, genesisConfig);
        loadGenesisConfig(genesisConfig);
    }

    virtual void loadConfigFromString(std::string const& _content)
    {
        boost::property_tree::ptree iniConfig;
        std::stringstream contentStream(_content);
        boost::property_tree::read_ini(contentStream, iniConfig);
        loadConfig(iniConfig);
    }

    virtual void loadGenesisConfigFromString(std::string const& _content)
    {
        boost::property_tree::ptree genesisConfig;
        std::stringstream contentStream(_content);
        boost::property_tree::read_ini(contentStream, genesisConfig);
        loadGenesisConfig(genesisConfig);
    }

    virtual void loadConfig(boost::property_tree::ptree const& _pt);
    virtual void loadGenesisConfig(boost::property_tree::ptree const& _genesisConfig);

    size_t txpoolLimit() const { return m_txpoolLimit; }
    size_t notifyWorkerNum() const { return m_notifyWorkerNum; }
    size_t verifierWorkerNum() const { return m_verifierWorkerNum; }

    bool smCryptoType() const { return m_smCryptoType; }
    std::string const& chainId() const { return m_chainId; }
    std::string const& groupId() const { return m_groupId; }
    size_t blockLimit() const { return m_blockLimit; }

    std::string const& privateKeyPath() const { return m_privateKeyPath; }

    size_t minSealTime() const { return m_minSealTime; }
    size_t checkPointTimeoutInterval() const { return m_checkPointTimeoutInterval; }

    std::string const& storagePath() const { return m_storagePath; }
    std::string const& storageDBName() const { return m_storageDBName; }
    std::string const& stateDBName() const { return m_stateDBName; }

    bcos::crypto::KeyFactory::Ptr keyFactory() { return m_keyFactory; }

    bcos::ledger::LedgerConfig::Ptr ledgerConfig() { return m_ledgerConfig; }

    std::string const& consensusType() const { return m_consensusType; }
    size_t txGasLimit() const { return m_txGasLimit; }
    std::string const& genesisData() const { return m_genesisData; }

    bool isWasm() const { return m_isWasm; }

    std::string const& rpcServiceName() const { return m_rpcServiceName; }
    std::string const& gatewayServiceName() const { return m_gatewayServiceName; }
    std::string const& groupManagerServiceName() const { return m_groupManagerServiceName; }

protected:
    virtual void loadTxPoolConfig(boost::property_tree::ptree const& _pt);
    virtual void loadChainConfig(boost::property_tree::ptree const& _pt);
    virtual void loadSecurityConfig(boost::property_tree::ptree const& _pt);
    virtual void loadSealerConfig(boost::property_tree::ptree const& _pt);

    virtual void loadStorageConfig(boost::property_tree::ptree const& _pt);
    virtual void loadConsensusConfig(boost::property_tree::ptree const& _pt);

    virtual void loadLedgerConfig(boost::property_tree::ptree const& _genesisConfig);

    void loadExecutorConfig(boost::property_tree::ptree const& _pt);

    void loadServiceConfig(boost::property_tree::ptree const& _pt);

private:
    bcos::consensus::ConsensusNodeListPtr parseConsensusNodeList(
        boost::property_tree::ptree const& _pt, std::string const& _sectionName,
        std::string const& _subSectionName);

    void generateGenesisData();
    virtual int64_t checkAndGetValue(boost::property_tree::ptree const& _pt,
        std::string const& _value, std::string const& _defaultValue);

private:
    bcos::crypto::KeyFactory::Ptr m_keyFactory;
    // txpool related configuration
    size_t m_txpoolLimit;
    size_t m_notifyWorkerNum;
    size_t m_verifierWorkerNum;
    // TODO: the block sync module need some configurations?

    // chain configuration
    bool m_smCryptoType;
    std::string m_chainId;
    std::string m_groupId;
    size_t m_blockLimit;

    // sealer configuration
    size_t m_minSealTime = 0;
    size_t m_checkPointTimeoutInterval;
    // for security
    std::string m_privateKeyPath;

    // ledger configuration
    std::string m_consensusType;
    bcos::ledger::LedgerConfig::Ptr m_ledgerConfig;
    size_t m_txGasLimit;
    std::string m_genesisData;

    // storage configuration
    std::string m_storagePath;
    std::string m_storageDBName = "storage";
    std::string m_stateDBName = "state";

    // executor config
    bool m_isWasm = false;

    std::string m_rpcServiceName;
    std::string m_gatewayServiceName;
    std::string m_groupManagerServiceName;
};
}  // namespace tool
}  // namespace bcos
