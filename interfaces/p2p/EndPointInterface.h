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
 * @brief interface for EndPoint
 * @file EndPointInterface.h
 * @author: yujiechen
 * @date 2021-04-08
 */
#pragma once
#include <bcos-framework/interfaces/crypto/KeyInterface.h>
namespace bcos
{
namespace p2p
{
class EndPointInterface
{
public:
    using Ptr = std::shared_ptr<EndPointInterface>;
    EndPointInterface() = default;
    virtual ~EndPointInterface() {}

    // ID of the node that sent the message
    virtual bcos::crypto::PublicPtr fromID() const = 0;
    // ID of the node receiving the message
    virtual bcos::crypto::PublicPtr toID() const = 0;

    // The ID of the gateway that sent the message
    virtual std::string const& fromZone() const = 0;
    // The ID of the gateway receiving the message
    virtual std::string const& toZone() const = 0;

    // the groupID
    virtual std::string const& groupID() const& = 0;

    virtual void setFromID(bcos::crypto::PublicPtr _fromID) = 0;
    virtual void setToID(bcos::crypto::PublicPtr _toID) = 0;
    virtual void setFromZone(std::string const& _fromZone) = 0;
    virtual void setToZone(std::string const& _toZone) = 0;
    virtual void setGroupID(std::string const& _groupID) = 0;
};
}  // namespace p2p
}  // namespace bcos