#pragma once

#include "../../libutilities/ConcurrentCOW.h"
#include "../protocol/ProtocolTypeDef.h"
#include "Common.h"

namespace bcos::storage
{
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
      : m_num(_num), m_data(std::map<std::string, std::string>())
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
        return m_data.get()->cbegin();
    }
    virtual std::map<std::string, std::string>::const_iterator end() const
    {
        return m_data.get()->cend();
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

    std::map<std::string, std::string>&& exportData() { return std::move(*m_data.mutableGet()); }

    void importData(std::map<std::string, std::string>&& input)
    {
        m_data.mutableGet()->swap(input);
    }

private:
    // should serialization
    protocol::BlockNumber m_num = 0;
    Status m_status = Status::NORMAL;
    bcos::ConcurrentCOW<std::map<std::string, std::string>> m_data;

    // no need to serialization
    ssize_t m_capacityOfHashField = 0;
    bool m_dirty = false;
    bool m_rollbacked = false;
};
}  // namespace bcos::storage