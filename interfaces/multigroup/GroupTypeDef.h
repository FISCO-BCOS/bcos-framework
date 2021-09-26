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
 * @brief define the basic type of the GroupManager
 * @file GroupTypeDef.h
 * @author: yujiechen
 * @date 2021-09-16
 */
#pragma once
#include "../../libutilities/Log.h"
#include <memory>

#define GROUP_LOG(LEVEL) BCOS_LOG(LEVEL) << LOG_BADGE("GROUP")
namespace bcos
{
namespace group
{
enum class GroupStatus : int32_t
{
    Creating = 0,
    Created = 1,
    Deleting = 2,
    Deleted = 3,
    Recovering = 4,
    // for the node status
    Starting = 5,
    Started = 6,
    Stopping = 7,
    Stopped = 8,
};

inline std::ostream& operator<<(std::ostream& _out, GroupStatus const& _er)
{
    switch (_er)
    {
    case GroupStatus::Creating:
        _out << "Creating";
        break;
    case GroupStatus::Created:
        _out << "Created";
        break;
    case GroupStatus::Deleting:
        _out << "Deleting";
        break;
    case GroupStatus::Deleted:
        _out << "Deleted";
        break;
    case GroupStatus::Starting:
        _out << "Starting";
        break;
    case GroupStatus::Started:
        _out << "Started";
        break;
    case GroupStatus::Stopping:
        _out << "Stopping";
        break;
    case GroupStatus::Stopped:
        _out << "Stopped";
        break;
    default:
        _out << "Unknown";
        break;
    }
    return _out;
}
}  // namespace group
}  // namespace bcos
