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
 * @file NodeConfig.cpp
 * @author: yujiechen
 * @date 2021-06-10
 */
#include "NodeConfig.h"
#include "../interfaces/consensus/ConsensusNode.h"
#include "../interfaces/ledger/LedgerTypeDef.h"
#include "../interfaces/protocol/ServiceDesc.h"
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>


#define MAX_BLOCK_LIMIT 5000

using namespace bcos;
using namespace bcos::crypto;
using namespace bcos::tool;
using namespace bcos::consensus;
using namespace bcos::ledger;
using namespace bcos::protocol;

NodeConfig::NodeConfig(KeyFactory::Ptr _keyFactory)
  : m_keyFactory(_keyFactory), m_ledgerConfig(std::make_shared<LedgerConfig>())
{}

void NodeConfig::loadConfig(boost::property_tree::ptree const& _pt)
{
    loadTxPoolConfig(_pt);
    loadChainConfig(_pt);
    loadSecurityConfig(_pt);
    loadSealerConfig(_pt);
    loadConsensusConfig(_pt);
    loadStorageConfig(_pt);
    loadExecutorConfig(_pt);
}

void NodeConfig::loadGenesisConfig(boost::property_tree::ptree const& _genesisConfig)
{
    loadLedgerConfig(_genesisConfig);
}

std::string NodeConfig::getServiceName(boost::property_tree::ptree const& _pt,
    std::string const& _configSection, std::string const& _objName,
    std::string const& _defaultValue, bool _require)
{
    auto serviceName = _pt.get<std::string>(_configSection, _defaultValue);
    if (!_require)
    {
        return serviceName;
    }
    checkService(_configSection, serviceName);
    return getPrxDesc(serviceName, _objName);
}

void NodeConfig::loadServiceConfig(boost::property_tree::ptree const& _pt)
{
    // rpc service name
    m_rpcServiceName = getServiceName(_pt, "service.rpc", RPC_SERVANT_NAME);
    // gateway service name
    m_gatewayServiceName = getServiceName(_pt, "service.gateway", GATEWAY_SERVANT_NAME);
    NodeConfig_LOG(INFO) << LOG_DESC("loadServiceConfig")
                         << LOG_KV("rpcServiceName", m_rpcServiceName)
                         << LOG_KV("gatewayServiceName", m_gatewayServiceName);
}

void NodeConfig::loadNodeServiceConfig(
    std::string const& _nodeID, boost::property_tree::ptree const& _pt)
{
    auto nodeName = _pt.get<std::string>("service.node_name", "");
    if (nodeName.size() == 0)
    {
        nodeName = _nodeID;
    }
    if (!isalNumStr(nodeName))
    {
        BOOST_THROW_EXCEPTION(
            InvalidConfig() << errinfo_comment("The node name must be number or digit"));
    }
    m_nodeName = nodeName;
    m_schedulerServiceName = getServiceName(_pt, "service.scheduler", SCHEDULER_SERVANT_NAME,
        getDefaultServiceName(nodeName, SCHEDULER_SERVICE_NAME), false);
    m_txpoolServiceName = getServiceName(_pt, "service.txpool", TXPOOL_SERVANT_NAME,
        getDefaultServiceName(nodeName, TXPOOL_SERVICE_NAME), false);
    m_consensusServiceName = getServiceName(_pt, "service.consensus", CONSENSUS_SERVANT_NAME,
        getDefaultServiceName(nodeName, CONSENSUS_SERVICE_NAME), false);
    m_frontServiceName = getServiceName(_pt, "service.front", FRONT_SERVANT_NAME,
        getDefaultServiceName(nodeName, FRONT_SERVICE_NAME), false);
    m_exeutorServiceName = getServiceName(_pt, "service.executor", EXECUTOR_SERVANT_NAME,
        getDefaultServiceName(nodeName, EXECUTOR_SERVICE_NAME), false);

    NodeConfig_LOG(INFO) << LOG_DESC("load node service") << LOG_KV("nodeName", m_nodeName)
                         << LOG_KV("schedulerServiceName", m_schedulerServiceName)
                         << LOG_KV("txpoolServiceName", m_txpoolServiceName)
                         << LOG_KV("consensusServiceName", m_consensusServiceName)
                         << LOG_KV("frontServiceName", m_frontServiceName)
                         << LOG_KV("exeutorServiceName", m_exeutorServiceName);
}
void NodeConfig::checkService(std::string const& _serviceType, std::string const& _serviceName)
{
    if (_serviceName.size() == 0)
    {
        BOOST_THROW_EXCEPTION(
            InvalidConfig() << errinfo_comment("Must set service name for " + _serviceType + "!"));
    }
    std::vector<std::string> serviceNameList;
    boost::split(serviceNameList, _serviceName, boost::is_any_of("."));
    std::string errorMsg =
        "Must set service name in format of application_name.server_name with only include letters "
        "and numbers for " +
        _serviceType + ", invalid config now is:" + _serviceName;
    if (serviceNameList.size() != 2)
    {
        BOOST_THROW_EXCEPTION(InvalidConfig() << errinfo_comment(errorMsg));
    }
    for (auto serviceName : serviceNameList)
    {
        if (!isalNumStr(serviceName))
        {
            BOOST_THROW_EXCEPTION(InvalidConfig() << errinfo_comment(errorMsg));
        }
    }
}


// load the txpool related params
void NodeConfig::loadTxPoolConfig(boost::property_tree::ptree const& _pt)
{
    m_txpoolLimit = checkAndGetValue(_pt, "txpool.limit", "15000");
    if (m_txpoolLimit <= 0)
    {
        BOOST_THROW_EXCEPTION(
            InvalidConfig() << errinfo_comment("Please set txpool.limit to positive !"));
    }
    m_notifyWorkerNum = checkAndGetValue(_pt, "txpool.notify_worker_num", "2");
    if (m_notifyWorkerNum <= 0)
    {
        BOOST_THROW_EXCEPTION(InvalidConfig() << errinfo_comment(
                                  "Please set txpool.notify_worker_num to positive !"));
    }

    m_verifierWorkerNum = checkAndGetValue(_pt, "txpool.verify_worker_num", "2");
    if (m_verifierWorkerNum <= 0)
    {
        BOOST_THROW_EXCEPTION(InvalidConfig() << errinfo_comment(
                                  "Please set txpool.verify_worker_num to positive !"));
    }
    NodeConfig_LOG(INFO) << LOG_DESC("loadTxPoolConfig") << LOG_KV("txpoolLimit", m_txpoolLimit)
                         << LOG_KV("notifierWorkers", m_notifyWorkerNum)
                         << LOG_KV("verifierWorkers", m_verifierWorkerNum);
}

void NodeConfig::loadChainConfig(boost::property_tree::ptree const& _pt)
{
    m_smCryptoType = _pt.get<bool>("chain.sm_crypto", false);
    m_groupId = _pt.get<std::string>("chain.group_id", "group");
    m_chainId = _pt.get<std::string>("chain.chain_id", "chain");
    if (!isalNumStr(m_chainId))
    {
        BOOST_THROW_EXCEPTION(
            InvalidConfig() << errinfo_comment("The chainId must be number or digit"));
    }
    m_blockLimit = checkAndGetValue(_pt, "chain.block_limit", "1000");
    if (m_blockLimit <= 0 || m_blockLimit > MAX_BLOCK_LIMIT)
    {
        BOOST_THROW_EXCEPTION(InvalidConfig() << errinfo_comment(
                                  "Please set chain.block_limit to positive and less than " +
                                  std::to_string(MAX_BLOCK_LIMIT) + " !"));
    }
    NodeConfig_LOG(INFO) << LOG_DESC("loadChainConfig") << LOG_KV("smCrypto", m_smCryptoType)
                         << LOG_KV("chainId", m_chainId) << LOG_KV("groupId", m_groupId)
                         << LOG_KV("blockLimit", m_blockLimit);
}

void NodeConfig::loadSecurityConfig(boost::property_tree::ptree const& _pt)
{
    m_privateKeyPath = _pt.get<std::string>("security.private_key_path", "node.pem");
    NodeConfig_LOG(INFO) << LOG_DESC("loadSecurityConfig")
                         << LOG_KV("privateKeyPath", m_privateKeyPath);
}

void NodeConfig::loadSealerConfig(boost::property_tree::ptree const& _pt)
{
    m_minSealTime = checkAndGetValue(_pt, "consensus.min_seal_time", "500");
    if (m_minSealTime <= 0)
    {
        BOOST_THROW_EXCEPTION(
            InvalidConfig() << errinfo_comment("Please set consensus.min_seal_time to positive!"));
    }
    NodeConfig_LOG(INFO) << LOG_DESC("loadSealerConfig") << LOG_KV("minSealTime", m_minSealTime);
}

void NodeConfig::loadStorageConfig(boost::property_tree::ptree const& _pt)
{
    m_storagePath = _pt.get<std::string>("storage.data_path", "data/" + m_groupId);
    NodeConfig_LOG(INFO) << LOG_DESC("loadStorageConfig") << LOG_KV("storagePath", m_storagePath);
}

void NodeConfig::loadConsensusConfig(boost::property_tree::ptree const& _pt)
{
    m_checkPointTimeoutInterval = checkAndGetValue(_pt, "consensus.checkpoint_timeout", "3000");
    if (m_checkPointTimeoutInterval < SYSTEM_CONSENSUS_TIMEOUT_MIN)
    {
        BOOST_THROW_EXCEPTION(InvalidConfig() << errinfo_comment(
                                  "Please set consensus.checkpoint_timeout to no less than " +
                                  std::to_string(SYSTEM_CONSENSUS_TIMEOUT_MIN) + "ms!"));
    }
    NodeConfig_LOG(INFO) << LOG_DESC("loadConsensusConfig")
                         << LOG_KV("checkPointTimeoutInterval", m_checkPointTimeoutInterval);
}

void NodeConfig::loadLedgerConfig(boost::property_tree::ptree const& _genesisConfig)
{
    // consensus type
    m_consensusType = _genesisConfig.get<std::string>("consensus.consensus_type", "pbft");
    // blockTxCountLimit
    auto blockTxCountLimit =
        checkAndGetValue(_genesisConfig, "consensus.block_tx_count_limit", "1000");
    if (blockTxCountLimit <= 0)
    {
        BOOST_THROW_EXCEPTION(InvalidConfig() << errinfo_comment(
                                  "Please set consensus.block_tx_count_limit to positive!"));
    }
    m_ledgerConfig->setBlockTxCountLimit(blockTxCountLimit);

    // consensusTimeout
    auto consensusTimeout = checkAndGetValue(_genesisConfig, "consensus.consensus_timeout", "3000");
    if (consensusTimeout < SYSTEM_CONSENSUS_TIMEOUT_MIN ||
        consensusTimeout >= SYSTEM_CONSENSUS_TIMEOUT_MAX)
    {
        BOOST_THROW_EXCEPTION(InvalidConfig() << errinfo_comment(
                                  "Please set consensus.consensus_timeout must between " +
                                  std::to_string(SYSTEM_CONSENSUS_TIMEOUT_MIN) + " and " +
                                  std::to_string(SYSTEM_CONSENSUS_TIMEOUT_MAX) + " !"));
    }
    m_minSealTime = std::min((int64_t)m_minSealTime, consensusTimeout);
    m_ledgerConfig->setConsensusTimeout(consensusTimeout);

    // txGasLimit
    auto txGasLimit = checkAndGetValue(_genesisConfig, "tx.gas_limit", "300000000");
    if (txGasLimit <= TX_GAS_LIMIT_MIN)
    {
        BOOST_THROW_EXCEPTION(
            InvalidConfig() << errinfo_comment(
                "Please set tx.gas_limit to more than " + std::to_string(TX_GAS_LIMIT_MIN) + " !"));
    }

    m_txGasLimit = txGasLimit;
    // sealerList
    auto consensusNodeList = parseConsensusNodeList(_genesisConfig, "consensus", "node.");
    if (!consensusNodeList || consensusNodeList->empty())
    {
        BOOST_THROW_EXCEPTION(InvalidConfig() << errinfo_comment("Must set sealerList!"));
    }
    m_ledgerConfig->setConsensusNodeList(*consensusNodeList);

    // leaderSwitchPeriod
    auto consensusLeaderPeriod = checkAndGetValue(_genesisConfig, "consensus.leader_period", "1");
    if (consensusLeaderPeriod <= 0)
    {
        BOOST_THROW_EXCEPTION(
            InvalidConfig() << errinfo_comment("Please set consensus.leader_period to positive!"));
    }
    m_ledgerConfig->setLeaderSwitchPeriod(consensusLeaderPeriod);
    NodeConfig_LOG(INFO) << LOG_DESC("loadLedgerConfig")
                         << LOG_KV("consensus_type", m_consensusType)
                         << LOG_KV("block_tx_count_limit", m_ledgerConfig->blockTxCountLimit())
                         << LOG_KV("consensus_timeout", m_ledgerConfig->consensusTimeout())
                         << LOG_KV("gas_limit", m_txGasLimit)
                         << LOG_KV("leader_period", m_ledgerConfig->leaderSwitchPeriod())
                         << LOG_KV("minSealTime", m_minSealTime);
    generateGenesisData();
}

ConsensusNodeListPtr NodeConfig::parseConsensusNodeList(boost::property_tree::ptree const& _pt,
    std::string const& _sectionName, std::string const& _subSectionName)
{
    if (!_pt.get_child_optional(_sectionName))
    {
        NodeConfig_LOG(DEBUG) << LOG_DESC("parseConsensusNodeList return for empty config")
                              << LOG_KV("sectionName", _sectionName);
        return nullptr;
    }
    auto nodeList = std::make_shared<ConsensusNodeList>();
    for (auto const& it : _pt.get_child(_sectionName))
    {
        if (it.first.find(_subSectionName) != 0)
        {
            continue;
        }
        std::string data = it.second.data();
        std::vector<std::string> nodeInfo;
        boost::split(nodeInfo, data, boost::is_any_of(":"));
        if (nodeInfo.size() == 0)
        {
            BOOST_THROW_EXCEPTION(
                InvalidConfig() << errinfo_comment(
                    "Uninitialized nodeInfo, key: " + it.first + ", value: " + data));
        }
        std::string nodeId = nodeInfo[0];
        boost::to_lower(nodeId);
        int64_t weight = 1;
        if (nodeInfo.size() == 2)
        {
            auto& weightInfoStr = nodeInfo[1];
            boost::trim(weightInfoStr);
            weight = boost::lexical_cast<int64_t>(weightInfoStr);
        }
        if (weight <= 0)
        {
            BOOST_THROW_EXCEPTION(InvalidConfig() << errinfo_comment(
                                      "Please set weight for " + nodeId + " to positive!"));
        }
        auto consensusNode = std::make_shared<ConsensusNode>(
            m_keyFactory->createKey(*fromHexString(nodeId)), weight);
        NodeConfig_LOG(INFO) << LOG_BADGE("parseConsensusNodeList")
                             << LOG_KV("sectionName", _sectionName) << LOG_KV("nodeId", nodeId)
                             << LOG_KV("weight", weight);
        nodeList->push_back(consensusNode);
    }
    NodeConfig_LOG(INFO) << LOG_BADGE("parseConsensusNodeList")
                         << LOG_KV("totalNodesSize", nodeList->size());
    return nodeList;
}

void NodeConfig::generateGenesisData()
{
    std::stringstream s;
    s << m_ledgerConfig->blockTxCountLimit() << "-" << m_ledgerConfig->consensusTimeout() << "-"
      << m_ledgerConfig->leaderSwitchPeriod() << "-" << m_txGasLimit << "-";
    for (auto node : m_ledgerConfig->consensusNodeList())
    {
        s << *toHexString(node->nodeID()->data()) << "," << node->weight() << ";";
    }
    m_genesisData = s.str();
    NodeConfig_LOG(INFO) << LOG_BADGE("generateGenesisData")
                         << LOG_KV("genesisData", m_genesisData);
}

void NodeConfig::loadExecutorConfig(boost::property_tree::ptree const& _pt)
{
    m_isWasm = _pt.get<bool>("executor.is_wasm", false);
    NodeConfig_LOG(INFO) << LOG_DESC("loadExecutorConfig") << LOG_KV("isWasm", m_isWasm);
}

// Note: make sure the consensus param checker is consistent with the precompiled param checker
int64_t NodeConfig::checkAndGetValue(boost::property_tree::ptree const& _pt,
    std::string const& _key, std::string const& _defaultValue)
{
    auto value = _pt.get<std::string>(_key, _defaultValue);
    try
    {
        return boost::lexical_cast<int64_t>(value);
    }
    catch (std::exception const& e)
    {
        BOOST_THROW_EXCEPTION(InvalidConfig() << errinfo_comment(
                                  "Invalid value " + value + " for configuration " + _key +
                                  ", please set the value with a valid number"));
    }
}