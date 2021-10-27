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
 * @brief typedef for gateway
 * @file GatewayTypeDef.h
 * @author: octopus
 * @date 2021-04-19
 */
#pragma once
#include <iostream>
#include <memory>
namespace bcos
{
namespace gateway
{
// Message type definition
enum MessageType : int16_t
{
    Heartbeat = 0x1,
    Handshake = 0x2,
    RequestNodeIDs = 0x3,
    ResponseNodeIDs = 0x4,
    PeerToPeerMessage = 0x5,
    BroadcastMessage = 0x6,
    AMOPMessageType = 0x7,
    WSMessageType = 0x8,
};
}  // namespace gateway
}  // namespace bcos
