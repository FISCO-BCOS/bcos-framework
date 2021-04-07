/*
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
 * @file Error.h
 * @author: yujiechen
 * @date: 2021-04-07
 */
#pragma once
#include <string>
namespace bcos
{
class Error
{
public:
    using Ptr = std::shared_ptr<Error>;
    Error() = default;
    Error(int64_t _errorCode, std::string const& _errorMessage)
      : m_errorCode(_errorCode), m_errorMessage(_errorMessage)
    {}

    virtual ~Error() {}

    virtual int64_t errorCode() const { return m_errorCode; }
    virtual std::string const& errorMessage() const { return m_errorMessage; }

    virtual void setErrorCode(int64_t _errorCode) { m_errorCode = _errorCode; }
    virtual void setErrorMessage(std::string const& _errorMessage)
    {
        m_errorMessage = _errorMessage;
    }

private:
    int64_t m_errorCode;
    std::string m_errorMessage;
};
}  // namespace bcos