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
 * @brief the information used to deploy new node
 * @file NodeInfo.h
 * @author: yujiechen
 * @date 2021-09-08
 */
#pragma once
#include "../../libutilities/Common.h"
#include "GroupTypeDef.h"
#include <memory>
namespace bcos
{
namespace group
{
enum NodeType : uint32_t
{
    NON_SM_NODE = 0,
    SM_NODE = 1,
};
class ChainNodeInfo
{
public:
    using Ptr = std::shared_ptr<ChainNodeInfo>;
    using ConstPtr = std::shared_ptr<const ChainNodeInfo>;
    using ServiceToDeployIpMap = std::map<std::string, std::string>;
    ChainNodeInfo() = default;
    ChainNodeInfo(std::string const& _nodeName, int32_t _type)
      : m_nodeName(_nodeName), m_nodeType((NodeType)_type), m_privateKey(std::make_shared<bytes>())
    {}
    virtual ~ChainNodeInfo() {}

    virtual std::string const& nodeName() const { return m_nodeName; }
    virtual NodeType const& nodeType() const { return m_nodeType; }
    virtual std::string const& deployIp(std::string const& _serviceName) const
    {
        if (!m_serviceToDeployIp.count(_serviceName))
        {
            return c_emptyIp;
        }
        return m_serviceToDeployIp.at(_serviceName);
    }
    virtual bytes const& privateKey() const { return *m_privateKey; }

    virtual void setNodeName(std::string const& _nodeName) { m_nodeName = _nodeName; }
    virtual void setNodeType(NodeType const& _nodeType) { m_nodeType = _nodeType; }
    virtual void setPrivateKey(bytes&& _privateKey) { *m_privateKey = std::move(_privateKey); }

    virtual void setPrivateKey(bytes const& _privateKey) { *m_privateKey = _privateKey; }

    virtual void appendDeployInfo(std::string const& _serviceName, std::string const& _deployIp)
    {
        m_serviceToDeployIp[_serviceName] = _deployIp;
    }

    virtual void setStatus(int32_t _status) { m_status = (GroupStatus)_status; }
    virtual GroupStatus status() const { return m_status; }

    virtual ServiceToDeployIpMap const& deployInfo() const { return m_serviceToDeployIp; }

    virtual void setDeployInfo(ServiceToDeployIpMap&& _deployInfo)
    {
        m_serviceToDeployIp = std::move(_deployInfo);
    }

    virtual void setIniConfig(std::string const& _iniConfig) { m_iniConfig = _iniConfig; }
    virtual std::string const& iniConfig() const { return m_iniConfig; }

private:
    // the node name
    std::string m_nodeName;
    NodeType m_nodeType;
    // mapping of service to deployed machine
    ServiceToDeployIpMap m_serviceToDeployIp;
    // the ini config maintained by the node, use the iniConfig of the node if empty
    std::string m_iniConfig = "";
    // the private key of the node
    bytesPointer m_privateKey;
    std::string c_emptyIp = "";
    GroupStatus m_status;
};
inline std::string printNodeInfo(ChainNodeInfo::Ptr _nodeInfo)
{
    if (!_nodeInfo)
    {
        return "";
    }
    std::stringstream oss;
    oss << LOG_KV("name", _nodeInfo->nodeName()) << LOG_KV("status", _nodeInfo->status())
        << LOG_KV("type", std::to_string((int32_t)_nodeInfo->nodeType()))
        << LOG_KV("deployedIps", _nodeInfo->deployInfo().size());
    return oss.str();
}
}  // namespace group
}  // namespace bcos