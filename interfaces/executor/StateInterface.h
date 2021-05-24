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
 * @brief interface of State
 * @file StateInterface.h
 * @author: xingqiangbai
 * @date: 2021-05-17
 */

#pragma once
#include "../../libutilities/Common.h"
#include "../../libutilities/FixedBytes.h"
#include "../crypto/CommonType.h"
#include "../protocol/ProtocolTypeDef.h"
#include "../storage/TableInterface.h"
#include <memory>

namespace bcos
{
namespace executor
{
class StateInterface : public std::enable_shared_from_this<StateInterface>
{
public:
    StateInterface() = default;
    virtual ~StateInterface() = default;
    /// Check if the address is in use.
    virtual bool addressInUse(const std::string& _address) const = 0;

    /// Check if the account exists in the state and is non empty (nonce > 0 || balance > 0 || code
    /// nonempty and suiside != 1). These two notions are equivalent after EIP158.
    virtual bool accountNonemptyAndExisting(const std::string& _address) const = 0;

    /// Check if the address contains executable code.
    virtual bool addressHasCode(const std::string& _address) const = 0;

    /// Get an account's balance.
    /// @returns 0 if the address has never been used.
    virtual u256 balance(const std::string& _address) const = 0;

    /// Add some amount to balance.
    /// Will initialise the address if it has never been used.
    virtual void addBalance(const std::string& _address, u256 const& _amount) = 0;

    /// Subtract the @p _value amount from the balance of @p _address account.
    /// @throws NotEnoughCash if the balance of the account is less than the
    /// amount to be subtrackted (also in case the account does not exist).
    virtual void subBalance(const std::string& _address, u256 const& _value) = 0;

    /// Set the balance of @p _address to @p _value.
    /// Will instantiate the address if it has never been used.
    virtual void setBalance(const std::string& _address, u256 const& _value) = 0;

    /**
     * @brief Transfers "the balance @a _value between two accounts.
     * @param _from Account from which @a _value will be deducted.
     * @param _to Account to which @a _value will be added.
     * @param _value Amount to be transferred.
     */
    virtual void transferBalance(
        const std::string& _from, const std::string& _to, u256 const& _value) = 0;

    /// Get the root of the storage of an account.
    virtual crypto::HashType storageRoot(const std::string& _address) const = 0;

    /// Get the value of a storage position of an account.
    /// @returns 0 if no account exists at that address.
    virtual u256 storage(const std::string& _address, const std::string_view& _memory) = 0;

    /// Set the value of a storage position of an account.
    virtual void setStorage(const std::string& _address, const std::string_view& _location,
        const std::string_view& _value) = 0;

    /// Clear the storage root hash of an account to the hash of the empty trie.
    virtual void clearStorage(const std::string& _address) = 0;

    /// Create a contract at the given address (with unset code and unchanged balance).
    virtual void createContract(const std::string& _address) = 0;

    /// Sets the code of the account. Must only be called during / after contract creation.
    virtual void setCode(const std::string& _address, bytesConstRef _code) = 0;

    /// Delete an account (used for processing suicides). (set suicides key = 1 when use AMDB)
    virtual void kill(const std::string& _address) = 0;

    /// Get the code of an account.
    /// @returns bytes() if no account exists at that address.
    /// @warning The reference to the code is only valid until the access to
    ///          other account. Do not keep it.
    virtual std::shared_ptr<bytes> code(const std::string& _address) const = 0;

    /// Get the code hash of an account.
    /// @returns EmptyHash if no account exists at that address or if there is no code
    /// associated with the address.
    virtual crypto::HashType codeHash(const std::string& _address) const = 0;

    /// Get the frozen status of an account.
    /// @returns ture if the account is frozen.
    virtual bool frozen(const std::string& _address) const = 0;

    /// Get the byte-size of the code of an account.
    /// @returns code(_address).size(), but utilizes CodeSizeHash.
    virtual size_t codeSize(const std::string& _address) const = 0;

    /// Increament the account nonce.
    virtual void incNonce(const std::string& _address) = 0;

    /// Set the account nonce.
    virtual void setNonce(const std::string& _address, u256 const& _newNonce) = 0;

    /// Get the account nonce -- the number of transactions it has sent.
    /// @returns 0 if the address has never been used.
    virtual u256 getNonce(const std::string& _address) const = 0;

    /// The hash of the root of our state tree.
    virtual crypto::HashType rootHash() const = 0;

    /// Commit all changes waiting in the address cache to the DB.
    /// @param _commitBehaviour whether or not to remove empty accounts during commit.
    virtual void commit() = 0;

    /// Resets any uncommitted changes to the cache. Return a new root in params &root
    virtual void setRoot(crypto::HashType const& _root) = 0;

    /// Get the account start nonce. May be required.
    virtual u256 const& accountStartNonce() const = 0;
    // virtual u256 const& requireAccountStartNonce() const = 0;
    // virtual void noteAccountStartNonce(u256 const& _actual) = 0;

    /// Create a savepoint in the state changelog.
    /// @return The savepoint index that can be used in rollback() function.
    virtual size_t savepoint() const = 0;

    /// Revert all recent changes up to the given @p _savepoint savepoint.
    virtual void rollback(size_t _savepoint) = 0;

    /// Clear state's cache
    virtual void clear() = 0;

    /// Check authority
    virtual bool checkAuthority(const std::string& _origin, const std::string& _address) const = 0;
};

}  // namespace executor
}  // namespace bcos
