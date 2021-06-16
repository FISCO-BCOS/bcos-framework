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
 * @file: TransactionStatus.cpp
 * @author: xingqiangbai
 * @date: 20200608
 */
#include "TransactionStatus.h"
#include "../interfaces/protocol/Exceptions.h"

using namespace std;
using namespace bcos;
using namespace bcos::protocol;

std::ostream& bcos::protocol::operator<<(std::ostream& _out, TransactionStatus const& _er)
{
    switch (_er)
    {
    case TransactionStatus::None:
        _out << "None";
        break;
    case TransactionStatus::OutOfGasLimit:
        _out << "OutOfGasLimit";
        break;
    case TransactionStatus::NotEnoughCash:
        _out << "NotEnoughCash";
        break;
    case TransactionStatus::BadInstruction:
        _out << "BadInstruction";
        break;
    case TransactionStatus::BadJumpDestination:
        _out << "BadJumpDestination";
        break;
    case TransactionStatus::OutOfGas:
        _out << "OutOfGas";
        break;
    case TransactionStatus::OutOfStack:
        _out << "OutOfStack";
        break;
    case TransactionStatus::StackUnderflow:
        _out << "StackUnderflow";
        break;
    case TransactionStatus::NonceCheckFail:
        _out << "NonceCheckFail";
        break;
    case TransactionStatus::BlockLimitCheckFail:
        _out << "BlockLimitCheckFail";
        break;
    case TransactionStatus::PrecompiledError:
        _out << "PrecompiledError";
        break;
    case TransactionStatus::RevertInstruction:
        _out << "RevertInstruction";
        break;
    case TransactionStatus::ContractAddressAlreadyUsed:
        _out << "ContractAddressAlreadyUsed";
        break;
    case TransactionStatus::PermissionDenied:
        _out << "PermissionDenied";
        break;
    case TransactionStatus::CallAddressError:
        _out << "CallAddressError";
        break;
    case TransactionStatus::GasOverflow:
        _out << "GasOverflow";
        break;
    case TransactionStatus::ContractFrozen:
        _out << "ContractFrozen";
        break;
    case TransactionStatus::AccountFrozen:
        _out << "AccountFrozen";
        break;
    case TransactionStatus::TxPoolIsFull:
        _out << "TxPoolIsFull";
        break;
    case TransactionStatus::Malform:
        _out << "MalformTx";
        break;
    case TransactionStatus::AlreadyInTxPool:
        _out << "AlreadyInTxPool";
        break;
    case TransactionStatus::TxAlreadyInChain:
        _out << "TxAlreadyInChain";
        break;
    case TransactionStatus::InvalidChainId:
        _out << "InvalidChainId";
        break;
    case TransactionStatus::InvalidGroupId:
        _out << "InvalidGroupId";
        break;
    case TransactionStatus::InvalidSignature:
        _out << "InvalidSignature";
        break;
    case TransactionStatus::RequestNotBelongToTheGroup:
        _out << "RequestNotBelongToTheGroup";
        break;
    default:
        _out << "Unknown";
        break;
    }
    return _out;
}
