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
 * @brief interface for GroupManager
 * @file GroupManagerInterface.h
 * @author: yujiechen
 * @date 2021-09-08
 */
#pragma once
#include "../../libutilities/Common.h"
#include "../../libutilities/Error.h"
#include "ChainNodeInfo.h"
#include "GroupInfo.h"

namespace bcos
{
namespace group
{
class GroupManagerInterface
{
public:
    using Ptr = std::shared_ptr<GroupManagerInterface>;
    using ConstPtr = std::shared_ptr<const GroupManagerInterface>;
    GroupManagerInterface() = default;
    virtual ~GroupManagerInterface() {}
    /**
     * @brief create a new group
     *
     * @param _groupInfo the information of the group to be created
     * @param _callback return the group creation result
     */
    virtual void asyncCreateGroup(
        GroupInfo::Ptr _groupInfo, std::function<void(Error::Ptr&&)> _callback) = 0;
    /**
     * @brief expand new node for the given group
     *
     * @param _chainID the chainID of the node
     * @param _groupID the groupID of the node
     * @param _nodeInfo the information of the new node to be created
     * @param _callback return group expansion result
     */
    virtual void asyncExpandGroupNode(std::string const& _chainID, std::string const& _groupID,
        ChainNodeInfo::Ptr _nodeInfo, std::function<void(Error::Ptr&&)> _callback) = 0;

    /**
     * @brief remove the given group from the given chain
     *
     * @param _chainID the chainID of the group to be removed
     * @param _groupID the groupID of the group to be removed
     * @param _callback return group remove result
     */
    virtual void asyncRemoveGroup(std::string const& _chainID, std::string const& _groupID,
        std::function<void(Error::Ptr&&)> _callback) = 0;

    /**
     * @brief remove the given node from the given group
     *
     * @param _chainID the chainID of the node to be removed
     * @param _groupID the gorupID of the node to be removed
     * @param _nodeName the name of the node to be removed
     * @param _callback return the node remove result
     */
    virtual void asyncRemoveGroupNode(std::string const& _chainID, std::string const& _groupID,
        std::string const& _nodeName, std::function<void(Error::Ptr&&)> _callback) = 0;

    /**
     * @brief recover the given group
     *
     * @param _chainID the chainID of the group to be recovered
     * @param _groupID the groupID of the group to be recovered
     * @param _callback return the group recovery result
     */
    virtual void asyncRecoverGroup(std::string const& _chainID, std::string const& _groupID,
        std::function<void(Error::Ptr&&)> _callback) = 0;

    /**
     * @brief recover the given node from the given group
     *
     * @param _chainID the chainID of the node to be recovered
     * @param _groupID the groupID of the node to be recoved
     * @param _nodeName the name of the node to be recovered
     * @param _callback return the node recover result
     */
    virtual void asyncRecoverGroupNode(std::string const& _chainID, std::string const& _groupID,
        std::string const& _nodeName, std::function<void(Error::Ptr&&)> _callback) = 0;

    /**
     * @brief start the given node of the given group
     *
     * @param _chainID the chainID of the node to be started
     * @param _groupID the groupID of the node to be started
     * @param _nodeName the name of the node to be started
     * @param _callback return the node start result
     */
    virtual void asyncStartNode(std::string const& _chainID, std::string const& _groupID,
        std::string const& _nodeName, std::function<void(Error::Ptr&&)> _callback) = 0;

    /**
     * @brief stop the given node
     *
     * @param _chainID the chainID of the node to be stopped
     * @param _groupID the groupID of the node to be stopped
     * @param _nodeName the name of the node to be stopped
     * @param _callback return the node stop result
     */
    virtual void asyncStopNode(std::string const& _chainID, std::string const& _groupID,
        std::string const& _nodeName, std::function<void(Error::Ptr&&)> _callback) = 0;

    /**
     * @brief get all the chainID maintained in this groupManager
     *
     * @param _onGetChainList return the chainID list
     */
    virtual void asyncGetChainList(
        std::function<void(Error::Ptr&&, std::set<std::string>&&)> _onGetChainList) = 0;

    /**
     * @brief get all the groupID list of a given chainID
     *
     * @param _chainID the chainID
     * @param _onGetGroupList return all the groupID list of a given chain
     */
    virtual void asyncGetGroupList(std::string _chainID,
        std::function<void(Error::Ptr&&, std::set<std::string>&&)> _onGetGroupList) = 0;

    /**
     * @brief get the group information of a given group
     *
     * @param _chainID the chainID of the group
     * @param _groupID the id of the group
     * @param _onGetGroupInfo return the queried group information
     */
    virtual void asyncGetGroupInfo(std::string _chainID, std::string _groupID,
        std::function<void(Error::Ptr&&, GroupInfo::Ptr&&)> _onGetGroupInfo) = 0;

    /**
     * @brief get the information of a given node
     *
     * @param _chainID the chainID of the node being queried
     * @param _groupID the groupID of the node being queried
     * @param _nodeName the name of the node being queried
     * @param _onGetNodeInfo return the node information
     */
    virtual void asyncGetNodeInfo(std::string _chainID, std::string _groupID, std::string _nodeName,
        std::function<void(Error::Ptr&&, ChainNodeInfo::Ptr&&)> _onGetNodeInfo) = 0;
};
}  // namespace group
}  // namespace bcos