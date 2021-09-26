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
 * @brief the information used to manager group
 * @file GroupInfo.h
 * @author: yujiechen
 * @date 2021-09-08
 */
#pragma once
#include "ChainNodeInfo.h"
#include "GroupTypeDef.h"
namespace bcos
{
namespace group
{
class GroupInfo
{
public:
    using Ptr = std::shared_ptr<GroupInfo>;
    GroupInfo() = default;
    GroupInfo(std::string const& _chainID, std::string const& _groupID)
      : m_chainID(_chainID), m_groupID(_groupID)
    {}
    virtual ~GroupInfo() {}

    virtual std::string const& genesisConfig() const { return m_genesisConfig; }
    virtual std::string const& iniConfig() const { return m_iniConfig; }
    virtual ChainNodeInfo::ConstPtr nodeInfo(std::string const& _nodeName) const
    {
        ReadGuard l(x_nodeInfos);
        if (!m_nodeInfos.count(_nodeName))
        {
            return nullptr;
        }
        return m_nodeInfos.at(_nodeName);
    }

    std::string const& groupID() const { return m_groupID; }
    std::string const& chainID() const { return m_chainID; }

    virtual void setGenesisConfig(std::string const& _genesisConfig)
    {
        m_genesisConfig = _genesisConfig;
    }
    virtual void setIniConfig(std::string const& _iniConfig) { m_iniConfig = _iniConfig; }
    virtual bool appendNodeInfo(ChainNodeInfo::Ptr _nodeInfo)
    {
        UpgradableGuard l(x_nodeInfos);
        auto const& nodeName = _nodeInfo->nodeName();
        if (m_nodeInfos.count(nodeName))
        {
            return false;
        }
        UpgradeGuard ul(l);
        m_nodeInfos[nodeName] = _nodeInfo;
        return true;
    }

    virtual void setGroupID(std::string const& _groupID) { m_groupID = _groupID; }
    virtual void setChainID(std::string const& _chainID) { m_chainID = _chainID; }

    virtual void setStatus(int32_t _status) { m_status = (GroupStatus)_status; }
    virtual GroupStatus status() const { return m_status; }

    virtual ssize_t nodesNum() const
    {
        ReadGuard l(x_nodeInfos);
        return m_nodeInfos.size();
    }

    // return copied nodeInfos to ensure thread-safe
    std::map<std::string, ChainNodeInfo::Ptr> nodeInfos() { return m_nodeInfos; }

private:
    std::string m_chainID;
    std::string m_groupID;
    // the genesis config for the group
    std::string m_genesisConfig;
    // the iniConfig for the group
    std::string m_iniConfig;
    // node name to node deployment information mapping
    std::map<std::string, ChainNodeInfo::Ptr> m_nodeInfos;
    mutable SharedMutex x_nodeInfos;
    // the group status
    GroupStatus m_status;
};

inline std::string printGroupInfo(GroupInfo::Ptr _groupInfo)
{
    if (!_groupInfo)
    {
        return "";
    }
    std::stringstream oss;
    oss << LOG_KV("group", _groupInfo->groupID()) << LOG_KV("chain", _groupInfo->chainID())
        << LOG_KV("status", _groupInfo->status()) << LOG_KV("nodeSize", _groupInfo->nodesNum());
    return oss.str();
}
}  // namespace group
}  // namespace bcos