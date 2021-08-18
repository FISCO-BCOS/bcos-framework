#pragma once

#include "../../libutilities/ConcurrentCOW.h"
#include "../protocol/ProtocolTypeDef.h"
#include "Common.h"

namespace std
{
inline bool operator<(const std::string_view& ls, const std::string& rs)
{
    return ls < std::string_view(rs);
}
inline bool operator<(const std::string& ls, const std::string_view& rs)
{
    return std::string_view(ls) < rs;
}
}  // namespace std

namespace bcos::storage
{
#ifdef USE_OLD_ENTRY
class Entry : public std::enable_shared_from_this<Entry>
{
public:
    enum Status
    {
        NORMAL = 0,
        DELETED = 1
    };
    using Ptr = std::shared_ptr<Entry>;
    using ConstPtr = std::shared_ptr<const Entry>;
    explicit Entry(protocol::BlockNumber _num = 0)
      : m_num(_num), m_data(std::make_shared<EntryData>())
    {
        m_data->m_refCount = 1;
    }
    virtual ~Entry()
    {
        RWMutexScoped lock(m_data->m_mutex, true);
        if (m_data->m_refCount > 0)
        {
            --m_data->m_refCount;
        }
    }

    virtual std::string getField(const std::string_view& key) const
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
    virtual std::string_view getFieldConst(const std::string_view& key) const
    {
        RWMutexScoped lock(m_data->m_mutex, false);
        auto it = m_data->m_fields.find(key);
        if (it != m_data->m_fields.end())
        {
            return it->second;
        }

        STORAGE_LOG(WARNING) << LOG_BADGE("Entry") << LOG_DESC("can't find")
                             << LOG_KV("field", key);
        return "";
    }
    virtual void setField(const std::string& key, const std::string& value)
    {
        setField(std::string(key), std::string(value));
    }

    virtual void setField(std::string&& key, std::string&& value)
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
            updatedCapacity = key.size() + value.size();
            m_data->m_fields.insert(
                std::make_pair(std::forward<std::string>(key), std::forward<std::string>(value)));
        }
        m_capacityOfHashField += updatedCapacity;
        m_dirty = true;
    }

    virtual std::map<std::string, std::string>::const_iterator find(
        const std::string_view& key) const
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

    virtual bool count(const std::string_view& key)
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
        m_status = entry->m_status;
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
    virtual void setDirty(bool dirty)
    {
        RWMutexScoped lock(m_data->m_mutex, true);
        m_dirty = dirty;
    }
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
        std::map<std::string, std::string, std::less<>> m_fields;
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

    bool m_dirty = false;
    bool m_rollbacked = false;
    ssize_t m_capacityOfHashField = 0;

    // should serialization
    protocol::BlockNumber m_num = 0;
    Status m_status = Status::NORMAL;
    EntryData::Ptr m_data;
};
#else
class Entry : public std::enable_shared_from_this<Entry>
{
public:
    enum Status
    {
        NORMAL = 0,
        DELETED = 1
    };
    using Ptr = std::shared_ptr<Entry>;
    using ConstPtr = std::shared_ptr<const Entry>;

    explicit Entry(protocol::BlockNumber _num = 0)
      : m_num(_num), m_data(std::map<std::string, std::string, std::less<>>())
    {}

    virtual ~Entry() {}

    std::string getField(const std::string_view& key) const
    {
        return std::string(getFieldConst(key));
    }

    std::string_view getFieldConst(const std::string_view& key) const
    {
        auto data = m_data.get();
        auto it = data->find(key);
        if (it != data->end())
        {
            return it->second;
        }
        STORAGE_LOG(ERROR) << LOG_BADGE("Entry") << LOG_DESC("can't find key")
                           << LOG_KV("key", key);
        return "";
    }
    void setField(const std::string& key, const std::string& value)
    {
        setField(std::string(key), std::string(value));
    }

    void setField(std::string&& key, std::string&& value)
    {
        ssize_t updatedCapacity = 0;
        auto data = m_data.mutableGet();
        auto it = data->find(key);
        if (it != data->end())
        {
            updatedCapacity = value.size() - it->second.size();
            it->second = value;
        }
        else
        {
            updatedCapacity = key.size() + value.size();
            data->insert(
                std::make_pair(std::forward<std::string>(key), std::forward<std::string>(value)));
        }
        m_capacityOfHashField += updatedCapacity;
        m_dirty = true;
    }

    virtual std::map<std::string, std::string>::const_iterator find(
        const std::string_view& key) const
    {
        return m_data.get()->find(key);
    }
    virtual std::map<std::string, std::string>::const_iterator begin() const
    {
        return m_data.get()->begin();
    }
    virtual std::map<std::string, std::string>::const_iterator end() const
    {
        return m_data.get()->end();
    }
    size_t size() const { return m_data.get()->size(); }

    bool rollbacked() const { return m_rollbacked; }
    void setRollbacked(bool _rollbacked) { m_rollbacked = _rollbacked; }
    Status getStatus() const { return m_status; }

    void setStatus(Status status)
    {
        (void)m_data.mutableGet();
        m_status = status;
        m_dirty = true;
    }

    bool count(const std::string_view& key) const
    {
        auto data = m_data.get();
        if (data->find(key) == data->end())
        {
            return false;
        }
        return true;
    }

    protocol::BlockNumber num() const { return m_num; }
    void setNum(protocol::BlockNumber num)
    {
        m_num = num;
        m_dirty = true;
    }

    void copyFrom(Entry::ConstPtr entry)
    {
        m_data = entry->m_data;
        m_num = entry->m_num;
        m_status = entry->m_status;
        m_dirty = entry->m_dirty;
        m_rollbacked = entry->m_rollbacked;
        m_capacityOfHashField = entry->m_capacityOfHashField;
    }

    bool dirty() const { return m_dirty; }
    void setDirty(bool dirty) { m_dirty = dirty; }
    ssize_t capacityOfHashField() const
    {  // the capacity is used to calculate gas, must return the same value in different DB
        return m_capacityOfHashField;
    }
    ssize_t refCount() const { return m_data.refCount(); }

private:
    ssize_t m_capacityOfHashField = 0;
    bool m_dirty = false;
    bool m_rollbacked = false;

    // should serialization
    protocol::BlockNumber m_num = 0;
    Status m_status = Status::NORMAL;
    // EntryData::Ptr m_data;
    bcos::ConcurrentCOW<std::map<std::string, std::string, std::less<>>> m_data;
};
#endif
}  // namespace bcos::storage