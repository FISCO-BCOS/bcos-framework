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
#include <variant>

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
    using ValueType = std::variant<std::string, std::vector<unsigned char>, std::vector<char>>;

    Entry() : m_data(EntryData()) {}

    explicit Entry(TableInfo::ConstPtr tableInfo, protocol::BlockNumber _num = 0)
      : m_data(EntryData()), m_tableInfo(std::move(tableInfo)), m_num(_num)
    {}

    Entry(const Entry&) = default;
    Entry(Entry&&) = default;
    bcos::storage::Entry& operator=(const Entry&) = default;
    bcos::storage::Entry& operator=(Entry&&) = default;

    virtual ~Entry() {}

    std::string_view getField(size_t index) const
    {
        auto& values = m_data.get()->values;
        if (index >= values.size())
        {
            BOOST_THROW_EXCEPTION(
                BCOS_ERROR(-1, "Get field index: " + boost::lexical_cast<std::string>(index) +
                                   " failed, index out of range: " +
                                   boost::lexical_cast<std::string>(values.size())));
        }

        return valueView(values[index]);
    }

    std::string_view getField(const std::string_view& field) const
    {
        if (!m_tableInfo)
        {
            BOOST_THROW_EXCEPTION(
                BCOS_ERROR(-1, "Get field: " + std::string(field) + " error, tableInfo is null"));
        }

        auto index = m_tableInfo->fieldIndex(field);
        return getField(index);
    }

    void setField(size_t index, ValueType value)
    {
        if (index >= m_data.get()->values.size())
        {
            BOOST_THROW_EXCEPTION(
                BCOS_ERROR(-1, "Set field index: " + boost::lexical_cast<std::string>(index) +
                                   " failed, index out of range: " +
                                   boost::lexical_cast<std::string>(m_data.get()->values.size())));
        }

        auto mutableData = m_data.mutableGet();
        auto& fieldValue = (mutableData->values)[index];

        auto view = valueView(value);
        ssize_t updatedCapacity = view.size() - std::get<0>(fieldValue).size();

        fieldValue = std::move(value);
        mutableData->capacityOfHashField += updatedCapacity;
        m_dirty = true;
    }

    void setField(const std::string_view& field, ValueType value)
    {
        if (!m_tableInfo)
        {
            BOOST_THROW_EXCEPTION(
                BCOS_ERROR(-1, "Set field: " + std::string(field) + " error, tableInfo is null"));
        }

        auto index = m_tableInfo->fieldIndex(field);

        auto data = m_data.mutableGet();
        if (data->values.size() < m_tableInfo->fields().size())
        {
            data->values.resize(m_tableInfo->fields().size());
        }

        auto& fieldValue = (data->values)[index];

        auto view = valueView(value);
        ssize_t updatedCapacity = view.size() - std::get<0>(fieldValue).size();
        fieldValue = std::move(value);
        data->capacityOfHashField += updatedCapacity;
        m_dirty = true;
    }

    auto begin() const noexcept
    {
        return boost::make_transform_iterator(m_data.get()->values.cbegin(),
            std::bind(&Entry::valueView, this, std::placeholders::_1));
    }
    auto end() const noexcept
    {
        return boost::make_transform_iterator(
            m_data.get()->values.cend(), std::bind(&Entry::valueView, this, std::placeholders::_1));
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
    void setNum(protocol::BlockNumber num) noexcept { m_num = num; }

    bool dirty() const noexcept { return m_dirty; }
    void setDirty(bool dirty) noexcept { m_dirty = dirty; }

    ssize_t capacityOfHashField() const noexcept
    {  // the capacity is used to calculate gas, must return the same value in different DB
        return m_data.get()->capacityOfHashField;
    }

    size_t version() const noexcept { return m_version; }
    void setVersion(size_t version) noexcept { m_version = version; }

    ssize_t refCount() const noexcept { return m_data.refCount(); }

    const std::vector<ValueType>& fields() const noexcept { return m_data.get()->values; }

    void importFields(std::vector<ValueType> values) noexcept
    {
        auto data = m_data.mutableGet();
        data->values.clear();
        data->capacityOfHashField = 0;
        for (auto& value : values)
        {
            auto view = valueView(value);
            data->capacityOfHashField += view.size();

            data->values.emplace_back(std::move(value));
        }

        m_dirty = true;
    }

    const std::vector<ValueType>&& exportFields() noexcept
    {
        auto data = m_data.mutableGet();
        data->capacityOfHashField = 0;
        return std::move(data->values);
    }

    TableInfo::ConstPtr tableInfo() const { return m_tableInfo; }
    void setTableInfo(TableInfo::ConstPtr tableInfo) { m_tableInfo = std::move(tableInfo); }

    bool valid() const noexcept { return ((m_status != Status::DELETED) && (!m_rollbacked)); }

private:
    struct EntryData
    {
        std::vector<ValueType> values;
        ssize_t capacityOfHashField = 0;
    };

    std::string_view valueView(const ValueType& value) const
    {
        switch (value.index())
        {
        case 0:
            return std::get<0>(value);
            break;
        case 1:
        {
            auto& data = std::get<1>(value);
            return std::string_view((char*)data.data(), data.size());
            break;
        }
        case 2:
        {
            auto& data = std::get<2>(value);
            return std::string_view((char*)data.data(), data.size());
            break;
        }
        }

        BOOST_THROW_EXCEPTION(BCOS_ERROR(UnknownEntryType,
            "Unknown entry type: " + boost::lexical_cast<std::string>(value.index())));
    }

    // should serialization
    Status m_status = Status::NORMAL;
    bcos::ConcurrentCOW<EntryData> m_data;

    // no need to serialization
    TableInfo::ConstPtr m_tableInfo;
    protocol::BlockNumber m_num = 0;
    size_t m_version = 0;
    bool m_dirty = false;
    bool m_rollbacked = false;
};
}  // namespace bcos::storage
