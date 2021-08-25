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
 * @brief interface of Table
 * @file Table.h
 * @author: xingqiangbai
 * @date: 2021-04-07
 */
#include "Table.h"
#include "../interfaces/storage/StorageInterface.h"
#include "tbb/concurrent_vector.h"
#include "tbb/enumerable_thread_specific.h"
#include "tbb/parallel_for.h"
#include "tbb/parallel_invoke.h"
#include <sstream>

using namespace std;

namespace bcos
{
namespace storage
{
Entry::Ptr Table::getRow(const std::string& _key)
{
    auto entryIt = m_cache.find(_key);
    if (entryIt != m_cache.end() && !entryIt->second->rollbacked())
    {
        if (entryIt->second->getStatus() != Entry::Status::DELETED)
        {
            auto entry = std::make_shared<Entry>();
            entry->copyFrom(entryIt->second);
            return entry;
        }
        // deleted
        return nullptr;
    }
    if (m_tableInfo->newTable)
    {  // new table has no data in DB
        return nullptr;
    }
    return m_DB->getRow(m_tableInfo, _key);
}

std::map<std::string, Entry::Ptr> Table::getRows(const std::vector<std::string>& _keys)
{
    std::map<std::string, Entry::Ptr> ret;
    std::map<std::string, Entry::Ptr> queryRet;
    tbb::parallel_invoke(
        [&]() {
            for (auto& key : _keys)
            {
                auto entryIt = m_cache.find(key);
                if (entryIt != m_cache.end() && !entryIt->second->rollbacked())
                {
                    if (entryIt->second->getStatus() != Entry::Status::DELETED)
                    {  // copy from
                        auto entry = std::make_shared<Entry>();
                        entry->copyFrom(entryIt->second);
                        ret[key] = entry;
                    }
                    else
                    {  // deleted
                        ret[key] = nullptr;
                    }
                }
                if (m_tableInfo->newTable)
                {  // new table has no data in DB
                    ret[key] = nullptr;
                }
            }
        },
        [&]() {
            if (!m_tableInfo->newTable)
            {  // new table has no data in DB
                queryRet = m_DB->getRows(m_tableInfo, _keys);
            }
        });

    ret.merge(queryRet);
    return ret;
}

std::vector<std::string> Table::getPrimaryKeys(const Condition::Ptr& _condition) const
{
    std::vector<std::string> ret;
    std::set<std::string> deleted;
    for (auto item : m_cache)
    {
        if (!item.second->rollbacked() && (!_condition || _condition->isValid(item.first)))
        {
            if (item.second->getStatus() != Entry::Status::DELETED)
            {
                ret.push_back(item.first);
            }
            else
            {
                deleted.insert(item.first);
            }
        }
    }
    if (m_tableInfo->newTable)
    {  // new table has no data in DB
        return ret;
    }
    auto len = ret.size();
    auto temp = m_DB->getPrimaryKeys(m_tableInfo, _condition);
    for (size_t i = 0; i < temp.size(); ++i)
    {
        if (!deleted.count(temp[i]) &&
            find(ret.begin(), ret.begin() + len, temp[i]) == ret.begin() + len)
        {
            ret.emplace_back(move(temp[i]));
        }
    }
    return ret;
}

bool Table::setRow(const std::string& _key, const Entry::Ptr& _entry)
{  // For concurrent_unordered_map, insert and emplace methods may create a temporary item that
    // is destroyed if another thread inserts an item with the same key concurrently.
    // So, parallel insert same key is not permitted
    if (!_entry)
    {
        STORAGE_LOG(ERROR) << LOG_BADGE("Table setRow empty entry") << LOG_KV("key", _key);
        return false;
    }
    // check entry fields
    for (auto& it : *_entry)
    {
        if (it.first != m_tableInfo->key &&
            m_tableInfo->fields.end() ==
                find(m_tableInfo->fields.begin(), m_tableInfo->fields.end(), it.first))
        {
            STORAGE_LOG(ERROR) << LOG_BADGE("Table") << LOG_DESC("invalid field")
                               << LOG_KV("table name", m_tableInfo->name)
                               << LOG_KV("field", it.first);
            return false;
        }
    }
    _entry->setNum(m_blockNumber);
    _entry->setField(m_tableInfo->key, _key);
    // get the old value, if the entry is not in m_cache, dont query db
    Entry::Ptr entry = nullptr;
    auto entryIt = m_cache.find(_key);
    if (entryIt != m_cache.end())
    {
        entry = entryIt->second;
        entryIt->second = _entry;
    }
    else
    {
        m_cache.insert(std::make_pair(_key, _entry));
    }

    m_recorder(make_shared<Change>(shared_from_this(), Change::Set, _key, entry, m_dataDirty));
    m_dataDirty = true;
    m_hashDirty = true;
    return true;
}

bool Table::remove(const std::string& _key)
{
    Entry::Ptr oldEntry = nullptr;

    auto entryIt = m_cache.find(_key);
    if (entryIt != m_cache.end())
    {
        // find in dirty, rollbacked means not exist in DB, Status::DELETED means it is deleted,
        // others mean the entry was modified
        if (!entryIt->second->rollbacked() &&
            entryIt->second->getStatus() != Entry::Status::DELETED)
        {
            entryIt->second->setStatus(Entry::Status::DELETED);
            entryIt->second->setNum(m_blockNumber);
            oldEntry = entryIt->second;
            STORAGE_LOG(DEBUG) << LOG_BADGE("Table remove in cache") << LOG_KV("key", _key);
        }
    }
    else
    {
        STORAGE_LOG(DEBUG) << LOG_BADGE("Table remove") << LOG_KV("key", _key);
        auto entry = std::make_shared<Entry>();
        entry->setNum(m_blockNumber);
        entry->setStatus(Entry::Status::DELETED);
        m_cache.insert(std::make_pair(_key, entry));
    }

    m_recorder(
        make_shared<Change>(shared_from_this(), Change::Remove, _key, oldEntry, m_dataDirty));
    m_hashDirty = true;
    m_dataDirty = true;

    return true;
}

void Table::asyncGetPrimaryKeys(const Condition::Ptr& _condition,
    std::function<void(const Error::Ptr&, const std::vector<std::string>&)> _callback)
{
    auto ret = make_shared<vector<string>>();
    auto deleted = make_shared<set<string>>();
    for (auto item : m_cache)
    {
        if (!item.second->rollbacked() && (!_condition || _condition->isValid(item.first)))
        {
            if (item.second->getStatus() != Entry::Status::DELETED)
            {
                ret->push_back(item.first);
            }
            else
            {
                deleted->insert(item.first);
            }
        }
    }
    if (m_tableInfo->newTable)
    {  // new table has no data in DB
        _callback(nullptr, *ret);
        return;
    }
    m_DB->asyncGetPrimaryKeys(m_tableInfo, _condition,
        [ret, deleted, _callback](const Error::Ptr& error, std::vector<std::string> keys) {
            auto len = ret->size();
            for (size_t i = 0; i < keys.size(); ++i)
            {
                if (!deleted->count(keys[i]) &&
                    find(ret->begin(), ret->begin() + len, keys[i]) == ret->begin() + len)
                {
                    ret->emplace_back(move(keys[i]));
                }
            }
            _callback(error, *ret);
        });
}

void Table::asyncGetRow(
    const std::string& _key, std::function<void(const Error::Ptr&, const Entry::Ptr&)> _callback)
{
    Entry::Ptr entry = nullptr;
    auto entryIt = m_cache.find(_key);
    if (entryIt != m_cache.end() && !entryIt->second->rollbacked())
    {
        if (entryIt->second->getStatus() != Entry::Status::DELETED)
        {
            entry = std::make_shared<Entry>();
            entry->copyFrom(entryIt->second);
            _callback(nullptr, entry);
            return;
        }
        _callback(make_shared<Error>(StorageErrorCode::NotFound, "the key was deleted"), entry);
        return;
    }
    if (m_tableInfo->newTable)
    {  // new table has no data in DB
        _callback(make_shared<Error>(StorageErrorCode::NotFound, "not found"), entry);
        return;
    }
    m_DB->asyncGetRow(m_tableInfo, _key, _callback);
}

void Table::asyncGetRows(const std::shared_ptr<std::vector<std::string>>& _keys,
    std::function<void(const Error::Ptr&, const std::map<std::string, Entry::Ptr>&)> _callback)
{
    auto ret = make_shared<std::map<std::string, Entry::Ptr>>();

    for (auto& key : *_keys)
    {
        auto entryIt = m_cache.find(key);
        if (entryIt != m_cache.end() && !entryIt->second->rollbacked())
        {
            if (entryIt->second->getStatus() != Entry::Status::DELETED)
            {  // copy from
                auto entry = std::make_shared<Entry>();
                entry->copyFrom(entryIt->second);
                (*ret)[key] = entry;
            }
            else
            {  // deleted
                (*ret)[key] = nullptr;
            }
        }
        if (m_tableInfo->newTable)
        {  // new table has no data in DB
            (*ret)[key] = nullptr;
        }
    }
    if (m_tableInfo->newTable)
    {  // new table has no data in DB
        _callback(nullptr, *ret);
        return;
    }
    m_DB->asyncGetRows(m_tableInfo, _keys,
        [ret, _callback](const Error::Ptr& error, std::map<std::string, Entry::Ptr> queryRet) {
            ret->merge(queryRet);
            _callback(error, *ret);
        });
}

crypto::HashType Table::hash()
{
    if (m_hashDirty && m_tableInfo->enableConsensus)
    {
        size_t totalBytes = 0;
        vector<Entry::Ptr> entryVec;
        bytes allData;
        auto data = dump(m_blockNumber);
#if FISCO_DEBUG
        stringstream ss;
        for (auto& entryIt : *data)
        {
            auto key = entryIt.first;
            auto entry = entryIt.second;
            ss << "[ status=" << entry->getStatus() << " ]"
               << "[ num=" << entry->num() << " ]"
               << "[ *key=" << key << " ]";
            for (auto& it : *entry)
            {
                ss << "[ " << it.first << "=" << it.second << " ]";
            }
        }
        STORAGE_LOG(DEBUG) << LOG_BADGE("FISCO_DEBUG") << LOG_KV("TableName", m_tableInfo->name)
                           << ss.str();
#endif
        if (data->size() != 0)
        {
            std::vector<size_t> entriesOffset;
            entriesOffset.reserve(data->size());
            entriesOffset.push_back(totalBytes);
            for (auto& it : *data)
            {
                entryVec.push_back(it.second);
                if (it.second->getStatus() != Entry::Status::DELETED)
                {
                    totalBytes += it.second->capacityOfHashField() + 1;  // 1 for status field
                }
                else
                {
                    totalBytes += it.first.size() + 1;
                }
                entriesOffset.push_back(totalBytes);
            }
            auto startT = utcTime();
            allData.resize(totalBytes, 0);
            // Parallel processing entries
            tbb::parallel_for(tbb::blocked_range<uint64_t>(0, entryVec.size()),
                [&](const tbb::blocked_range<uint64_t>& range) {
                    for (uint64_t i = range.begin(); i < range.end(); i++)
                    {
                        auto entry = entryVec[i];
                        auto startOffSet = entriesOffset[i];
                        if (entry->getStatus() != Entry::Status::DELETED)
                        {
                            for (auto& fieldIt : *(entry))
                            {
                                if (isHashField(fieldIt.first))
                                {
                                    memcpy(&allData[startOffSet], &fieldIt.first[0],
                                        fieldIt.first.size());
                                    startOffSet += fieldIt.first.size();
                                    memcpy(&allData[startOffSet], &fieldIt.second[0],
                                        fieldIt.second.size());
                                    startOffSet += fieldIt.second.size();
                                }
                            }
                        }
                        else
                        {
                            auto key = entry->getFieldConst(m_tableInfo->key);
                            memcpy(&allData[startOffSet], key.data(), key.size());
                        }
                        char status = (char)entry->getStatus();
                        memcpy(&allData[startOffSet], &status, sizeof(status));
                    }
                });

            auto writeDataT = utcTime() - startT;
            startT = utcTime();
            bytesConstRef bR(allData.data(), allData.size());
            auto transDataT = utcTime() - startT;
            startT = utcTime();
            m_hash = m_hashImpl->hash(bR);
            auto getHashT = utcTime() - startT;
            STORAGE_LOG(TRACE) << LOG_BADGE("Table hash calculate")
                               << LOG_KV("table", m_tableInfo->name)
                               << LOG_KV("writeDataT", writeDataT)
                               << LOG_KV("transDataT", transDataT) << LOG_KV("getHashT", getHashT)
                               << LOG_KV("hash", m_hash.abridged());
        }
        m_hashDirty = false;
        return m_hash;
    }
    STORAGE_LOG(DEBUG) << LOG_BADGE("Table hash use cache") << LOG_KV("table", m_tableInfo->name);
    return m_hash;
}

void Table::rollback(Change::Ptr _change)
{
    switch (_change->kind)
    {
    case Change::Set:
    {
        if (_change->entry)
        {
            m_cache[_change->key] = _change->entry;
        }
        else
        {  // nullptr means the key is not exist in m_cache
            auto oldEntry = std::make_shared<Entry>();
            oldEntry->setNum(m_blockNumber);
            oldEntry->setRollbacked(true);
            m_cache[_change->key] = oldEntry;
        }
        m_hashDirty = true;
        m_dataDirty = _change->tableDirty;
        break;
    }
    case Change::Remove:
    {
        m_cache[_change->key]->setStatus(Entry::Status::NORMAL);
        if (_change->entry)
        {
            m_cache[_change->key] = _change->entry;
        }
        else
        {
            m_cache[_change->key]->setRollbacked(true);
        }
        m_hashDirty = true;
        m_dataDirty = _change->tableDirty;
        break;
    }
    default:
        break;
    }
}

}  // namespace storage
}  // namespace bcos
