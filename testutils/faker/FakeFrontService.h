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
 * @brief fake frontService
 * @file FakeFrontService.h
 * @author: yujiechen
 * @date 2021-05-25
 */
#pragma once

#include "../../interfaces/front/FrontServiceInterface.h"
#include "../../interfaces/txpool/TxPoolInterface.h"
using namespace bcos;
using namespace bcos::front;
using namespace bcos::crypto;
using namespace bcos::protocol;
using namespace bcos::txpool;

namespace bcos
{
namespace test
{
class FakeFrontService : public FrontServiceInterface
{
public:
    using Ptr = std::shared_ptr<FakeFrontService>;
    explicit FakeFrontService(NodeIDPtr _nodeId) : m_nodeId(_nodeId) {}

    ~FakeFrontService() override {}

    void start() override {}
    void stop() override {}
    void addTxPool(NodeIDPtr _nodeId, TxPoolInterface::Ptr _txpool)
    {
        m_nodeId2TxPool[_nodeId] = _txpool;
    }
    // TODO: for txs sync
    void asyncGetNodeIDs(GetNodeIDsFunc) override {}
    // for gateway: useless here
    void onReceiveNodeIDs(
        const std::string&, std::shared_ptr<const NodeIDs>, ReceiveMsgFunc) override
    {}
    // for gateway: useless here
    void onReceiveMessage(const std::string&, NodeIDPtr, bytesConstRef, ReceiveMsgFunc) override {}
    // useless for sync/pbft/txpool
    void asyncSendMessageByNodeIDs(int, const std::vector<NodeIDPtr>&, bytesConstRef) override {}
    // useless for sync/pbft/txpool
    void asyncSendBroadcastMessage(int, bytesConstRef) override {}
    // useless for sync/pbft/txpool
    void onReceiveBroadcastMessage(
        const std::string&, NodeIDPtr, bytesConstRef, ReceiveMsgFunc) override
    {}
    void asyncSendResponse(const std::string&, bytesConstRef) override {}

    void asyncSendMessageByNodeID(int _moduleId, NodeIDPtr _nodeId, bytesConstRef _data, uint32_t,
        CallbackFunc _responseCallback) override
    {
        if (_moduleId == ModuleID::TxsSync && m_nodeId2TxPool.count(_nodeId))
        {
            auto txpool = m_nodeId2TxPool[_nodeId];
            auto bytesData = std::make_shared<bytes>(_data.begin(), _data.end());
            txpool->asyncNotifyTxsSyncMessage(
                nullptr, m_nodeId, bytesData,
                [_responseCallback, _nodeId](bytesConstRef _respData) {
                    // called when receive response data
                    _responseCallback(nullptr, _nodeId, _respData, "", nullptr);
                },
                nullptr);
        }
        if (m_nodeId2AsyncSendSize.count(_nodeId))
        {
            m_nodeId2AsyncSendSize[_nodeId]++;
        }
        else
        {
            m_nodeId2AsyncSendSize[_nodeId] = 1;
        }

        m_totalSendMsgSize++;
    }

    size_t getAsyncSendSizeByNodeID(NodeIDPtr _nodeId)
    {
        if (!m_nodeId2AsyncSendSize.count(_nodeId))
        {
            return 0;
        }
        return m_nodeId2AsyncSendSize[_nodeId];
    }

    size_t totalSendMsgSize() { return m_totalSendMsgSize; }

private:
    size_t m_totalSendMsgSize = 0;
    NodeIDPtr m_nodeId;
    std::map<NodeIDPtr, size_t, KeyCompare> m_nodeId2AsyncSendSize;
    std::map<NodeIDPtr, TxPoolInterface::Ptr, KeyCompare> m_nodeId2TxPool;
};
}  // namespace test
}  // namespace bcos