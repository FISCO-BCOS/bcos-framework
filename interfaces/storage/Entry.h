#pragma once

#include "../../libutilities/Common.h"
#include "../../libutilities/ConcurrentCOW.h"
#include "../protocol/ProtocolTypeDef.h"
#include "Common.h"
#include <boost/exception/diagnostic_information.hpp>
#include <boost/throw_exception.hpp>
#include <exception>

namespace bcos::storage
{
class Entry
{
public:
    enum Status
    {
        NORMAL = 0,
        DELETED = 1
    };
    using Ptr = std::shared_ptr<Entry>;
    using ConstPtr = std::shared_ptr<const Entry>;

    explicit Entry(const TableInfo::ConstPtr& tableInfo, protocol::BlockNumber _num = 0) noexcept
      : m_num(_num),
        m_data(Data{tableInfo, std::vector<std::string>(tableInfo->field2Index.size()), 0})
    {}

    explicit Entry(const Entry& entry) noexcept = default;
    explicit Entry(Entry&& entry) noexcept = default;
    bcos::storage::Entry& operator=(const Entry& entry) noexcept = default;
    bcos::storage::Entry& operator=(Entry&& entry) noexcept = default;

    virtual ~Entry() noexcept {}

    std::string getField(const std::string_view& key) const
    {
        return std::string(getFieldConst(key));
    }

    std::string_view getFieldConst(const std::string_view& key) const
    {
        try
        {
            auto& field = constField(key);
            return field;
        }
        catch (const std::exception& e)
        {
            STORAGE_LOG(ERROR) << "getFieldConst failed " << boost::diagnostic_information(e);
        }

        return "";
    }

    const std::string& constField(const std::string_view& key) const
    {
        size_t index;
        auto& tableInfo = m_data.get()->tableInfo;
        auto indexIt = tableInfo->field2Index.find(key);
        if (indexIt != tableInfo->field2Index.end())
        {
            index = indexIt->second;
            auto& field = (m_data.get()->fields)[index];
            return field;
        }
        else
        {
            BOOST_THROW_EXCEPTION(bcos::Exception("Can't find field: " + std::string(key)));
        }
    }

    void setField(const std::string_view& key, const std::string& value)
    {
        setField(key, std::string(value));
    }

    void setField(const std::string_view& key, std::string&& value)
    {
        try
        {
            ssize_t updatedCapacity = 0;

            auto& field = mutableField(key);
            updatedCapacity = value.size() - field.size();
            field = std::move(value);
            m_data.mutableGet()->capacityOfHashField += updatedCapacity;
            m_dirty = true;
        }
        catch (const std::exception& e)
        {
            STORAGE_LOG(ERROR) << "setField failed " << boost::diagnostic_information(e);
        }
    }

    std::string& mutableField(const std::string_view& key)
    {
        auto& tableInfo = m_data.get()->tableInfo;
        auto indexIt = tableInfo->field2Index.find(key);
        if (indexIt != tableInfo->field2Index.end())
        {
            size_t index = indexIt->second;
            auto& field = (m_data.mutableGet()->fields)[index];
            return field;
        }
        else
        {
            BOOST_THROW_EXCEPTION(bcos::Exception("Can't find field: " + std::string(key)));
        }
    }

    virtual std::vector<std::string>::const_iterator begin() const noexcept
    {
        return m_data.get()->fields.cbegin();
    }
    virtual std::vector<std::string>::const_iterator end() const noexcept
    {
        return m_data.get()->fields.cend();
    }

    bool rollbacked() const noexcept { return m_rollbacked; }
    void setRollbacked(bool _rollbacked) noexcept { m_rollbacked = _rollbacked; }
    Status getStatus() const noexcept { return m_status; }

    void setStatus(Status status) noexcept
    {
        (void)m_data.mutableGet();
        m_status = status;
        m_dirty = true;
    }

    protocol::BlockNumber num() const noexcept { return m_num; }
    void setNum(protocol::BlockNumber num) noexcept
    {
        m_num = num;
        m_dirty = true;
    }

    bool dirty() const noexcept { return m_dirty; }
    void setDirty(bool dirty) noexcept { m_dirty = dirty; }

    ssize_t capacityOfHashField() const noexcept
    {  // the capacity is used to calculate gas, must return the same value in different DB
        return m_data.get()->capacityOfHashField;
    }

    size_t version() const noexcept { return m_version; }
    void setVersion(size_t version) noexcept { m_version = version; }

    ssize_t refCount() const noexcept { return m_data.refCount(); }

    std::vector<std::string>&& exportData() noexcept
    {
        return std::move(m_data.mutableGet()->fields);
    }

    void importData(std::vector<std::string>&& input) noexcept
    {
        m_data.mutableGet()->fields = std::move(input);
    }

private:
    struct Data
    {
        TableInfo::ConstPtr tableInfo;
        std::vector<std::string> fields;
        ssize_t capacityOfHashField;
    };

    // should serialization
    protocol::BlockNumber m_num = 0;
    Status m_status = Status::NORMAL;
    bcos::ConcurrentCOW<Data> m_data;

    // no need to serialization
    size_t m_version = 0;
    bool m_dirty = false;
    bool m_rollbacked = false;
};
}  // namespace bcos::storage
