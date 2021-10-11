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
#include <json/json.h>
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
      : m_nodeName(_nodeName), m_nodeType((NodeType)_type)
    {}

    explicit ChainNodeInfo(std::string const& _jsonGroupInfoStr) { deserialize(_jsonGroupInfoStr); }

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
    virtual std::string const& privateKey() const { return m_privateKey; }

    virtual void setNodeName(std::string const& _nodeName) { m_nodeName = _nodeName; }
    virtual void setNodeType(NodeType const& _nodeType) { m_nodeType = _nodeType; }
    virtual void setPrivateKey(std::string&& _privateKey) { m_privateKey = std::move(_privateKey); }

    virtual void setPrivateKey(std::string const& _privateKey) { m_privateKey = _privateKey; }

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

protected:
    virtual void deserialize(std::string const& _jsonNodeInfo)
    {
        Json::Value value;
        Json::Reader jsonReader;
        auto ret = jsonReader.parse(_jsonNodeInfo, value);
        if (!ret)
        {
            BOOST_THROW_EXCEPTION(InvalidChainNodeInfo() << errinfo_comment(
                                      "The chain node information must be valid json string."));
        }
        // required: parse nodeNmae
        if (!value.isMember("name"))
        {
            BOOST_THROW_EXCEPTION(
                InvalidChainNodeInfo() << errinfo_comment("Must set the chain node name."));
        }
        setNodeName(value["name"].asString());
        // required: parse nodeType
        if (!value.isMember("type"))
        {
            BOOST_THROW_EXCEPTION(
                InvalidChainNodeInfo() << errinfo_comment("Must set the chain node type."));
        }
        NodeType type = (NodeType)(value["type"].asUInt());
        setNodeType(type);
        // required: parse deployInfo
        if (!value.isMember("deploy"))
        {
            BOOST_THROW_EXCEPTION(
                InvalidChainNodeInfo() << errinfo_comment("Must contain the deploy info."));
        }
        if (!value["deploy"].isArray())
        {
            BOOST_THROW_EXCEPTION(
                InvalidChainNodeInfo() << errinfo_comment("The deploy info must be array."));
        }
        auto const& deployInfo = value["deploy"];
        for (Json::ArrayIndex i = 0; i < deployInfo.size(); i++)
        {
            auto const& deployItem = deployInfo[i];
            if (!deployItem.isObject() || !deployItem.isMember("service") ||
                !deployItem.isMember("ip"))
            {
                BOOST_THROW_EXCEPTION(
                    InvalidChainNodeInfo() << errinfo_comment(
                        "Invalid deploy info: must contain the service name and the ip"));
            }
            appendDeployInfo(deployItem["service"].asString(), deployItem["ip"].asString());
        }
        // optional: parse privateKey
        if (value.isMember("privateKey"))
        {
            setPrivateKey(value["privateKey"].asString());
        }
        // optional: parse status
        if (value.isMember("status"))
        {
            setStatus((NodeType)(value["status"].asInt()));
        }
        // optional: parse iniConfig
        if (value.isMember("ini"))
        {
            setIniConfig(value["ini"].asString());
        }
    }

private:
    // the node name
    std::string m_nodeName;
    NodeType m_nodeType;
    // mapping of service to deployed machine
    ServiceToDeployIpMap m_serviceToDeployIp;
    // the ini config maintained by the node, use the iniConfig of the node if empty
    std::string m_iniConfig = "";
    // the private key of the node in pem format
    std::string m_privateKey;
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