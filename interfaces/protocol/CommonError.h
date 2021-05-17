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
 * @brief define common error for all the modules
 * @file CommonError.h
 * @author: yujiechen
 * @date 2021-04-22
 */
#pragma once
#include <stdint.h>
namespace bcos
{
namespace protocol
{
enum CommonError : int32_t
{
    SUCCESS = 0,
    TIMEOUT = 1000,
    TransactionsMissing = 2000,  // for transaction sync
    InconsistentTransactions = 2001,
    TxsSignatureVerifyFailed = 2002,
    FetchTransactionsFailed = 2003,
};
}
}  // namespace bcos