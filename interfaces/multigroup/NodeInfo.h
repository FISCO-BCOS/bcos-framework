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
class NodeInfo
{
public:
    using Ptr = std::shared_ptr<NodeInfo>;
    using ConstPtr = std::shared_ptr<const NodeInfo>;
    using ServiceToDeployIpMap = std::map<std::string, std::string>;
    NodeInfo() = default;
    virtual ~NodeInfo() {}

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

private:
    // the node name
    std::string m_nodeName;
    NodeType m_nodeType;
    // mapping of service to deployed machine
    ServiceToDeployIpMap m_serviceToDeployIp;
    // the private key of the node
    bytesPointer m_privateKey;
    std::string const c_emptyIp = "";
};
}  // namespace group
}  // namespace bcos