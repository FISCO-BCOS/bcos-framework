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
#include <boost/throw_exception.hpp>
#include <sstream>

using namespace std;

namespace bcos
{
namespace storage
{
Entry::Ptr Table::getRow(const std::string& _key)
{
    std::promise<std::tuple<Error::Ptr, Entry::Ptr>> promise;

    asyncGetRow(_key, [&promise](Error::Ptr&& error, Entry::Ptr&& entry) {
        promise.set_value(std::tuple{std::move(error), std::move(entry)});
    });

    auto result = promise.get_future().get();

    if (std::get<0>(result))
    {
        BOOST_THROW_EXCEPTION(*(std::get<0>(result)));
    }

    return std::get<1>(result);
}

std::vector<Entry::Ptr> Table::getRows(const gsl::span<std::string>& _keys)
{
    std::promise<std::tuple<Error::Ptr, std::vector<Entry::Ptr>>> promise;
    asyncGetRows(_keys, [&promise](Error::Ptr&& error, std::vector<Entry::Ptr>&& entries) {
        promise.set_value(std::tuple{std::move(error), std::move(entries)});
    });

    auto result = promise.get_future().get();

    if (std::get<0>(result))
    {
        BOOST_THROW_EXCEPTION(*(std::get<0>(result)));
    }

    return std::get<1>(result);
}

std::vector<std::string> Table::getPrimaryKeys(const Condition::Ptr& _condition)
{
    std::promise<std::tuple<Error::Ptr, std::vector<std::string>>> promise;
    asyncGetPrimaryKeys(
        _condition, [&promise](Error::Ptr&& error, std::vector<std::string>&& keys) {
            promise.set_value(std::tuple{std::move(error), std::move(keys)});
        });
    auto result = promise.get_future().get();

    if (std::get<0>(result))
    {
        BOOST_THROW_EXCEPTION(*(std::get<0>(result)));
    }

    return std::get<1>(result);
}

bool Table::setRow(const std::string& _key, const Entry::Ptr& _entry)
{
    std::promise<std::tuple<Error::Ptr, bool>> promise;
    m_storage->asyncSetRow(m_tableInfo, _key, _entry, [&promise](Error::Ptr&& error, bool success) {
        promise.set_value(std::tuple{error, success});
    });
    auto result = promise.get_future().get();

    if (std::get<0>(result))
    {
        BOOST_THROW_EXCEPTION(*(std::get<0>(result)));
    }

    return std::get<1>(result);
}

bool Table::remove(const std::string& _key)
{
    std::promise<std::tuple<Error::Ptr, bool>> promise;
    m_storage->asyncRemove(m_tableInfo, _key, [&promise](Error::Ptr&& error, bool success) {
        promise.set_value(std::tuple{error, success});
    });
    auto result = promise.get_future().get();

    if (std::get<0>(result))
    {
        BOOST_THROW_EXCEPTION(*(std::get<0>(result)));
    }

    return std::get<1>(result);
}

void Table::asyncGetPrimaryKeys(const Condition::Ptr& _condition,
    std::function<void(Error::Ptr&&, std::vector<std::string>&&)> _callback) noexcept
{
    m_storage->asyncGetPrimaryKeys(m_tableInfo, _condition, _callback);
}

void Table::asyncGetRow(
    const std::string& _key, std::function<void(Error::Ptr&&, Entry::Ptr&&)> _callback) noexcept
{
    m_storage->asyncGetRow(m_tableInfo, _key, _callback);
}

void Table::asyncGetRows(const gsl::span<std::string>& _keys,
    std::function<void(Error::Ptr&&, std::vector<Entry::Ptr>&&)> _callback) noexcept
{
    m_storage->asyncGetRows(m_tableInfo, _keys, _callback);
}

/*
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
                ss << it << ", ";
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
                totalBytes += it.second->capacityOfHashField() + 1;  // 1 for status field
                entriesOffset.push_back(totalBytes);
            }
            auto startT = utcTime();
            allData.resize(totalBytes);
            // Parallel processing entries
            tbb::parallel_for(tbb::blocked_range<uint64_t>(0, entryVec.size()),
                [&](const tbb::blocked_range<uint64_t>& range) {
                    for (uint64_t i = range.begin(); i < range.end(); i++)
                    {
                        auto entry = entryVec[i];
                        auto startOffSet = entriesOffset[i];

                        for (auto& value : *(entry))
                        {
                            if (!value.empty())
                            {
                                memcpy(&allData[startOffSet], value.data(), value.size());
                                startOffSet += value.size();
                            }
                        }

                        char status = (char)entry->getStatus();
                        allData[startOffSet] = status;
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
*/

}  // namespace storage
}  // namespace bcos
