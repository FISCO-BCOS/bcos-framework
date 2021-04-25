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
#pragma once

#include "../../interfaces/protocol/ProtocolTypeDef.h"
#include "../../libutilities/Log.h"
#include "tbb/spin_mutex.h"
#include "tbb/spin_rw_mutex.h"
#include "tbb/tbb_thread.h"
#include <map>
#include <memory>
#include <string>
#include <vector>

#define STORAGE_LOG(LEVEL) LOG(LEVEL) << "[STORAGE]"


namespace bcos
{
namespace storage
{
class Condition : public std::enable_shared_from_this<Condition>
{
public:
    using Ptr = std::shared_ptr<Condition>;
    //
    virtual void NE(const std::string& value) { m_conditions.emplace_back(Comparator::NE, value); }
    virtual void GT(const std::string& value) { m_conditions.emplace_back(Comparator::GT, value); }
    virtual void GE(const std::string& value) { m_conditions.emplace_back(Comparator::GE, value); }
    virtual void LT(const std::string& value) { m_conditions.emplace_back(Comparator::LT, value); }
    virtual void LE(const std::string& value) { m_conditions.emplace_back(Comparator::LE, value); }
    virtual void limit(size_t start, size_t end)
    {
        m_limit = std::pair<size_t, size_t>(start, end);
    }

    virtual std::pair<size_t, size_t> getLimit() const { return m_limit; }

    virtual bool isValid(const std::string_view& key) const
    {  // all conditions must be satisfied
        for (auto& cond : m_conditions)
        {  // conditions should few, so not parallel check for now
            switch (cond.cmp)
            {
            case Comparator::NE:
                if (key == cond.value)
                {
                    return false;
                }
                break;
            case Comparator::GT:
                if (key <= cond.value)
                {
                    return false;
                }
                break;
            case Comparator::GE:
                if (key < cond.value)
                {
                    return false;
                }
                break;
            case Comparator::LT:
                if (key >= cond.value)
                {
                    return false;
                }
                break;
            case Comparator::LE:
                if (key > cond.value)
                {
                    return false;
                }
                break;
            default:
                // undefined Comparator
                break;
            }
        }
        return true;
    }

private:
    enum class Comparator
    {
        EQ,
        NE,
        GT,
        GE,
        LT,
        LE,
    };
    struct cond
    {
        cond(Comparator _cmp, const std::string& _value) : cmp(_cmp), value(_value) {}
        Comparator cmp;
        std::string value;
    };
    std::vector<cond> m_conditions;
    std::pair<size_t, size_t> m_limit;
};

struct TableInfo : public std::enable_shared_from_this<TableInfo>
{
    using Ptr = std::shared_ptr<TableInfo>;
    std::string name;
    std::string key;
    std::vector<std::string> fields;
    // std::vector<Address> authorizedAddress;
    std::vector<std::string> indices;

    bool enableConsensus = true;
    bool enableCache = true;
};


class Entry : public std::enable_shared_from_this<Entry>
{
public:
    enum Status
    {
        NORMAL = 0,
        DELETED
    };
    using Ptr = std::shared_ptr<Entry>;
    using ConstPtr = std::shared_ptr<const Entry>;
    Entry() : m_data(std::make_shared<EntryData>()) { m_data->m_refCount = 1; }
    virtual ~Entry()
    {
        RWMutexScoped lock(m_data->m_mutex, true);
        if (m_data->m_refCount > 0)
        {
            --m_data->m_refCount;
        }
    }

    virtual std::string getField(const std::string& key) const
    {
        RWMutexScoped lock(m_data->m_mutex, false);
        auto it = m_data->m_fields.find(key);
        if (it != m_data->m_fields.end())
        {
            return it->second;
        }
        STORAGE_LOG(ERROR) << LOG_BADGE("Entry") << LOG_DESC("can't find key")
                           << LOG_KV("key", key);
        return "";
    }
    virtual std::string_view getFieldConst(const std::string& key) const
    {
        RWMutexScoped lock(m_data->m_mutex, false);
        auto it = m_data->m_fields.find(key);
        if (it != m_data->m_fields.end())
        {
            return it->second;
        }

        STORAGE_LOG(ERROR) << LOG_BADGE("Entry") << LOG_DESC("can't find key")
                           << LOG_KV("key", key);
        return "";
    }
    virtual void setField(const std::string& key, const std::string_view& value)
    {
        auto lock = checkRef();

        auto it = m_data->m_fields.find(key);
        ssize_t updatedCapacity = 0;
        if (it != m_data->m_fields.end())
        {
            updatedCapacity = value.size() - it->second.size();
            it->second = value;
        }
        else
        {
            m_data->m_fields.insert(std::make_pair(key, value));
            updatedCapacity = key.size() + value.size();
        }
        m_capacityOfHashField += updatedCapacity;
        m_dirty = true;
    }
    virtual std::map<std::string, std::string>::const_iterator find(const std::string& key) const
    {
        return m_data->m_fields.find(key);
    }
    virtual std::map<std::string, std::string>::const_iterator begin() const
    {
        return m_data->m_fields.begin();
    }
    virtual std::map<std::string, std::string>::const_iterator end() const
    {
        return m_data->m_fields.end();
    }
    virtual size_t size() const { return m_data->m_fields.size(); }

    virtual bool rollbacked() const
    {
        RWMutexScoped lock(m_data->m_mutex, false);
        return m_rollbacked;
    }
    virtual void setRollbacked(bool _rollbacked)
    {
        RWMutexScoped lock(m_data->m_mutex, true);
        m_rollbacked = _rollbacked;
    }
    virtual Status getStatus() const
    {
        RWMutexScoped lock(m_data->m_mutex, false);

        return m_status;
    }

    virtual void setStatus(Status status)
    {
        auto lock = checkRef();
        m_status = status;
        m_dirty = true;
    }

    virtual bool count(const std::string& key)
    {
        if (m_data->m_fields.find(key) == m_data->m_fields.end())
        {
            return false;
        }
        return true;
    }

    virtual protocol::BlockNumber num() const
    {
        RWMutexScoped lock(m_data->m_mutex, false);
        return m_num;
    }
    virtual void setNum(protocol::BlockNumber num)
    {
        RWMutexScoped lock(m_data->m_mutex, true);

        m_num = num;
        m_dirty = true;
    }

    virtual void copyFrom(Entry::ConstPtr entry)
    {
        RWMutexScoped lock(m_data->m_mutex, true);

        RWMutexScoped lock2;
        while (true)
        {
            auto locked = lock2.try_acquire(entry->m_data->m_mutex, true);
            if (!locked)
            {
                if (m_data == entry->m_data)
                {
                    return;
                }
                else
                {
                    tbb::this_tbb_thread::yield();
                }
            }
            else
            {
                break;
            }
        }

        m_num = entry->m_num;
        m_dirty = entry->m_dirty;
        m_rollbacked = entry->m_rollbacked;
        m_capacityOfHashField = entry->m_capacityOfHashField;

        auto oldData = m_data;
        m_data->m_refCount -= 1;

        m_data = entry->m_data;
        lock.release();

        m_data->m_refCount += 1;
    }
    virtual bool dirty() const { return m_dirty; }

    virtual ssize_t capacityOfHashField() const
    {  // the capacity is used to calculate gas, must return the same value in different DB
        RWMutexScoped lock(m_data->m_mutex, false);
        return m_capacityOfHashField;
    }
    // TODO: just for test?
    virtual ssize_t refCount() const
    {
        RWMutexScoped lock(m_data->m_mutex, false);
        return m_data->m_refCount;
    }

private:
    typedef tbb::spin_rw_mutex RWMutex;
    typedef tbb::spin_rw_mutex::scoped_lock RWMutexScoped;

    struct EntryData
    {
        typedef std::shared_ptr<EntryData> Ptr;

        EntryData(){};

        ssize_t m_refCount = 0;
        std::map<std::string, std::string> m_fields;
        RWMutex m_mutex;
    };
    std::shared_ptr<RWMutexScoped> checkRef()
    {
        auto lock = std::make_shared<RWMutexScoped>(m_data->m_mutex, true);

        if (m_data->m_refCount > 1)
        {
            auto m_oldData = m_data;
            m_data = std::make_shared<EntryData>();

            m_data->m_refCount = 1;
            m_data->m_fields = m_oldData->m_fields;
            m_oldData->m_refCount -= 1;
            assert(m_oldData->m_refCount >= 0);
            lock = std::make_shared<tbb::spin_rw_mutex::scoped_lock>(m_data->m_mutex, true);
        }

        return lock;
    }

    protocol::BlockNumber m_num = 0;
    bool m_dirty = false;
    Status m_status = Status::NORMAL;
    bool m_rollbacked = false;
    ssize_t m_capacityOfHashField = 0;

    EntryData::Ptr m_data;
};
inline bool isHashField(const std::string_view& _key)
{
    if (!_key.empty())
    {
        return ((_key.substr(0, 1) != "_" && _key.substr(_key.size() - 1, 1) != "_"));
    }
    return false;
}
}  // namespace storage
}  // namespace bcos
