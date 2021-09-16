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
 * @brief factory to build the chainInfo
 * @file ChainInfoFactory.h
 * @author: yujiechen
 * @date 2021-09-18
 */
#pragma once
#include "ChainInfo.h"
namespace bcos
{
namespace group
{
class ChainInfoFactory
{
public:
    using Ptr = std::shared_ptr<ChainInfoFactory>;
    ChainInfoFactory() = default;
    virtual ~ChainInfoFactory() {}
    virtual ChainInfo::Ptr createChainInfo(std::string const& _chainID)
    {
        return std::make_shared<ChainInfo>(_chainID);
    }
};
}  // namespace group
}  // namespace bcos