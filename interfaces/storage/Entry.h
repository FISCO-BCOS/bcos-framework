#pragma once

#include "../../libutilities/Common.h"
#include "../../libutilities/ConcurrentCOW.h"
#include "../../libutilities/Error.h"
#include "../protocol/ProtocolTypeDef.h"
#include "Common.h"
#include <boost/exception/diagnostic_information.hpp>
#include <boost/lexical_cast.hpp>
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

    explicit Entry() noexcept : m_num(0), m_data(Data{nullptr, std::vector<std::string>(), 0}) {}

    explicit Entry(const TableInfo::ConstPtr& tableInfo, protocol::BlockNumber _num = 0) noexcept
      : m_num(_num)
    {
        if (tableInfo)
        {
            m_data.reset(Data{tableInfo, std::vector<std::string>(tableInfo->fields.size()), 0});
        }
        else
        {
            m_data.reset(Data{nullptr, std::vector<std::string>(), 0});
        }
    }

    explicit Entry(const Entry& entry) noexcept = default;
    explicit Entry(Entry&& entry) noexcept = default;
    bcos::storage::Entry& operator=(const Entry& entry) noexcept = default;
    bcos::storage::Entry& operator=(Entry&& entry) noexcept = default;

    virtual ~Entry() noexcept {}

    std::string_view getField(size_t index) const
    {
        auto& fields = m_data.get()->fields;
        if (index >= fields.size())
        {
            BOOST_THROW_EXCEPTION(
                BCOS_ERROR(-1, "Get field index: " + boost::lexical_cast<std::string>(index) +
                                   " failed, index out of range: " +
                                   boost::lexical_cast<std::string>(fields.size())));
        }

        return fields[index];
    }

    std::string_view getField(const std::string_view& field) const
    {
        auto& tableInfo = m_data.get()->tableInfo;
        if (!tableInfo)
        {
            BOOST_THROW_EXCEPTION(
                BCOS_ERROR(-1, "Get field: " + std::string(field) + " error, tableInfo is null"));
        }

        auto indexIt = tableInfo->field2Index.find(field);
        if (indexIt != tableInfo->field2Index.end())
        {
            auto index = indexIt->second;
            return (m_data.get()->fields)[index];
        }
        else
        {
            BOOST_THROW_EXCEPTION(BCOS_ERROR(-1, "Can't find field: " + std::string(field)));
        }
    }

    void setField(size_t index, const std::string& value) { setField(index, std::string(value)); }

    void setField(size_t index, std::string&& value)
    {
        if (index >= m_data.get()->fields.size())
        {
            BOOST_THROW_EXCEPTION(
                BCOS_ERROR(-1, "Set field index: " + boost::lexical_cast<std::string>(index) +
                                   " failed, index out of range: " +
                                   boost::lexical_cast<std::string>(m_data.get()->fields.size())));
        }

        ssize_t updatedCapacity = value.size() - value.size();
        auto data = m_data.mutableGet();
        data->fields[index] = std::move(value);
        data->capacityOfHashField += updatedCapacity;
        m_dirty = true;
    }

    void setField(const std::string_view& key, const std::string& value)
    {
        setField(key, std::string(value));
    }

    void setField(const std::string_view& key, std::string&& value)
    {
        auto& tableInfo = m_data.get()->tableInfo;

        if (!tableInfo)
        {
            BOOST_THROW_EXCEPTION(
                BCOS_ERROR(-1, "Set field: " + std::string(key) + " error, tableInfo is null"));
        }

        auto indexIt = tableInfo->field2Index.find(key);
        if (indexIt == tableInfo->field2Index.end())
        {
            BOOST_THROW_EXCEPTION(BCOS_ERROR(-1, "Can't find field: " + std::string(key)));
        }
        size_t index = indexIt->second;

        auto data = m_data.mutableGet();
        auto& field = (data->fields)[index];

        ssize_t updatedCapacity = value.size() - field.size();
        field = std::move(value);
        data->capacityOfHashField += updatedCapacity;
        m_dirty = true;
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
    Status status() const noexcept { return m_status; }

    void setStatus(Status status) noexcept
    {
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

    const std::vector<std::string>& fields() const noexcept { return m_data.get()->fields; }

    void importFields(std::vector<std::string>&& input) noexcept
    {
        m_data.mutableGet()->fields = std::move(input);
    }

    std::vector<std::string>&& exportFields() noexcept
    {
        return std::move(m_data.mutableGet()->fields);
    }

    TableInfo::ConstPtr tableInfo() const { return m_data.get()->tableInfo; }

    bool valid() const noexcept { return ((m_status != Status::DELETED) && (!m_rollbacked)); }

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
