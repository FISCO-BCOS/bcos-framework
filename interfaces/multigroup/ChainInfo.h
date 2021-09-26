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
 * @brief the information for the chain
 * @file ChainInfo.h
 * @author: yujiechen
 * @date 2021-09-08
 */
#pragma once
#include "../../libutilities/Common.h"
#include "GroupTypeDef.h"
namespace bcos
{
namespace group
{
class ChainInfo
{
public:
    using Ptr = std::shared_ptr<ChainInfo>;
    explicit ChainInfo(std::string const& _chainID) : m_chainID(_chainID) {}
    virtual ~ChainInfo() {}

    virtual bool appendGroup(std::string const& _groupID)
    {
        return appendData(x_groupList, m_groupList, _groupID);
    }

    virtual bool removeGroup(std::string const& _groupID)
    {
        return removeData(x_groupList, m_groupList, _groupID);
    }

    virtual bool appendService(std::string const& _serviceName)
    {
        return appendData(x_serviceList, m_serviceList, _serviceName);
    }

    virtual bool removeService(std::string const& _serviceName)
    {
        return removeData(x_serviceList, m_serviceList, _serviceName);
    }

    virtual std::string const& chainID() const { return m_chainID; }
    virtual std::set<std::string> const& groupList() const { return m_groupList; }
    virtual std::set<std::string> const& serviceList() const { return m_serviceList; }
    virtual GroupStatus status() const { return m_status; }
    virtual void setStatus(GroupStatus _status) { m_status = _status; }

    virtual void setGroupList(std::set<std::string>&& _groupList)
    {
        WriteGuard l(x_groupList);
        m_groupList = std::move(_groupList);
    }

    virtual void setServiceList(std::set<std::string>&& _serviceList)
    {
        WriteGuard l(x_serviceList);
        m_serviceList = std::move(_serviceList);
    }

protected:
    bool appendData(SharedMutex& _lock, std::set<std::string>& _dataList, std::string const& _data)
    {
        UpgradableGuard l(_lock);
        if (_dataList.count(_data))
        {
            return false;
        }
        UpgradeGuard ul(l);
        _dataList.insert(_data);
        return true;
    }

    bool removeData(SharedMutex& _lock, std::set<std::string>& _dataList, std::string const& _data)
    {
        UpgradableGuard l(_lock);
        if (!_dataList.count(_data))
        {
            return false;
        }
        UpgradeGuard ul(l);
        _dataList.erase(_data);
        return true;
    }

private:
    std::string m_chainID;
    std::set<std::string> m_groupList;
    mutable SharedMutex x_groupList;

    std::set<std::string> m_serviceList;
    mutable SharedMutex x_serviceList;

    GroupStatus m_status;
};

inline std::string printChainInfo(ChainInfo::Ptr _chainInfo)
{
    if (!_chainInfo)
    {
        return "";
    }
    std::stringstream oss;
    oss << LOG_KV("id", _chainInfo->chainID()) << LOG_KV("status", _chainInfo->status())
        << LOG_KV("groupNum", _chainInfo->groupList().size());
    return oss.str();
}
}  // namespace group
}  // namespace bcos