#pragma once

#include "../../libutilities/Common.h"
#include "../../libutilities/ConcurrentCOW.h"
#include "../../libutilities/Error.h"
#include "../protocol/ProtocolTypeDef.h"
#include "Common.h"
#include <boost/exception/diagnostic_information.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/any_range.hpp>
#include <boost/throw_exception.hpp>
#include <exception>
#include <initializer_list>
#include <variant>

namespace bcos::storage
{
class Entry
{
public:
    enum Status : int8_t
    {
        NORMAL = 0,
        DELETED = 1
    };
    using ValueType = std::variant<std::string, std::vector<unsigned char>, std::vector<char>>;

    Entry() : m_data(EntryData()) {}

    explicit Entry(TableInfo::ConstPtr tableInfo)
      : m_data(EntryData()), m_tableInfo(std::move(tableInfo))
    {}

    Entry(const Entry&) noexcept = default;
    Entry(Entry&&) noexcept = default;
    bcos::storage::Entry& operator=(const Entry&) noexcept = default;
    bcos::storage::Entry& operator=(Entry&&) noexcept = default;

    ~Entry() noexcept {}

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

        int32_t updatedCapacity = valueView(value).size() - valueView(fieldValue).size();

        fieldValue = std::move(value);
        m_capacityOfHashField += updatedCapacity;
        m_dirty = true;
    }

    void setField(const std::string_view& field, ValueType value)
    {
        if (!m_tableInfo)
        {
            BOOST_THROW_EXCEPTION(
                BCOS_ERROR(-1, "Set field: " + std::string(field) + " error, tableInfo is null"));
        }

        auto data = m_data.mutableGet();
        if (data->values.size() < m_tableInfo->fields().size())
        {
            data->values.resize(m_tableInfo->fields().size());
        }

        auto index = m_tableInfo->fieldIndex(field);
        setField(index, std::move(value));
    }

    auto begin() const
    {
        return boost::make_transform_iterator(m_data.get()->values.cbegin(),
            std::bind(&Entry::valueView, this, std::placeholders::_1));
    }
    auto end() const
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

    bool dirty() const noexcept { return m_dirty; }
    void setDirty(bool dirty) noexcept { m_dirty = dirty; }

    ssize_t capacityOfHashField() const noexcept
    {  // the capacity is used to calculate gas, must return the same value in different DB
        return m_capacityOfHashField;
    }

    ssize_t refCount() const { return m_data.refCount(); }

    auto&& fields() const noexcept { return m_data.get()->values; }

    void importFields(std::initializer_list<ValueType> values)
    {
        EntryData data;
        data.values.reserve(values.size());
        m_capacityOfHashField = 0;

        for (auto& value : values)
        {
            m_capacityOfHashField += valueView(value).size();
            data.values.emplace_back(std::move(value));
        }

        m_data.reset(std::move(data));
        m_dirty = true;
    }

    void importFields(std::vector<std::string> values)
    {
        EntryData data;
        data.values.reserve(values.size());
        m_capacityOfHashField = 0;

        for (auto& value : values)
        {
            m_capacityOfHashField += value.size();
            data.values.emplace_back(std::move(value));
        }

        m_data.reset(std::move(data));
        m_dirty = true;
    }

    auto&& exportFields()
    {
        auto data = m_data.mutableGet();
        m_capacityOfHashField = 0;
        return std::move(data->values);
    }

    TableInfo::ConstPtr tableInfo() const { return m_tableInfo; }
    void setTableInfo(TableInfo::ConstPtr tableInfo) { m_tableInfo = std::move(tableInfo); }

    bool valid() const noexcept { return ((m_status != Status::DELETED) && (!m_rollbacked)); }

private:
    std::string_view valueView(const ValueType& value) const
    {
        std::string_view view;
        std::visit(
            [&view](
                auto&& value) { view = std::string_view((const char*)value.data(), value.size()); },
            value);

        return view;
    }

    struct EntryData
    {
        boost::container::small_vector<ValueType, 1> values;
    };

    bcos::ConcurrentCOW<EntryData> m_data;  // should serialization
    TableInfo::ConstPtr m_tableInfo;        // no need to serialization
    int32_t m_capacityOfHashField = 0;      // no need to serialization
    Status m_status = Status::NORMAL;       // should serialization
    bool m_dirty = false;                   // no need to serialization
    bool m_rollbacked = false;              // no need to serialization
};
}  // namespace bcos::storage
