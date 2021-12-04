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
#include <algorithm>
#include <cstdint>
#include <exception>
#include <initializer_list>
#include <type_traits>
#include <variant>

namespace bcos::storage
{
class Entry
{
public:
    enum Status : int8_t
    {
        NORMAL = 0,
        DELETED,
        PURGED,
    };

    constexpr static int32_t SMALL_SIZE = 32;
    constexpr static int32_t MEDIUM_SIZE = 64;
    constexpr static int32_t LARGE_SIZE = INT32_MAX;

    using SBOBuffer = std::array<bcos::byte, SMALL_SIZE>;

    using ValueType = std::variant<SBOBuffer, std::string, std::vector<unsigned char>,
        std::vector<char>, std::shared_ptr<std::string>,
        std::shared_ptr<std::vector<unsigned char>>, std::shared_ptr<std::vector<char>>>;

    Entry() = default;

    explicit Entry(TableInfo::ConstPtr) {}

    Entry(const Entry&) = default;
    Entry(Entry&&) noexcept = default;
    bcos::storage::Entry& operator=(const Entry&) = default;
    bcos::storage::Entry& operator=(Entry&&) noexcept = default;

    ~Entry() noexcept {}

    std::string_view getField(size_t index) const
    {
        if (index > 0)
        {
            BOOST_THROW_EXCEPTION(
                BCOS_ERROR(-1, "Get field index: " + boost::lexical_cast<std::string>(index) +
                                   " failed, index out of range"));
        }

        return valueView(m_value);
    }

    template <typename T>
    void setField(size_t index, T value)
    {
        if (index > 0)
        {
            BOOST_THROW_EXCEPTION(
                BCOS_ERROR(-1, "Set field index: " + boost::lexical_cast<std::string>(index) +
                                   " failed, index out of range"));
        }

        auto view = valueView(value);
        m_size = view.size();
        if (m_size <= SMALL_SIZE)
        {
            if (m_value.index() != 0)
            {
                m_value = SBOBuffer();
            }

            std::copy_n(view.data(), view.size(), std::get<0>(m_value).data());
        }
        else if (m_size <= MEDIUM_SIZE)
        {
            m_value = std::move(value);
        }
        else
        {
            m_value = std::make_shared<T>(std::move(value));
        }

        m_dirty = true;
    }

    auto begin() const { return &m_value; }
    auto end() const { return &m_value + 1; }

    Status status() const noexcept { return m_status; }

    void setStatus(Status status) noexcept
    {
        m_status = status;
        m_dirty = true;
    }

    bool dirty() const noexcept { return m_dirty; }
    void setDirty(bool dirty) noexcept { m_dirty = dirty; }

    int32_t size() const noexcept { return m_size; }

    void importFields(std::initializer_list<ValueType> values)
    {
        if (values.size() > 1)
        {
            BOOST_THROW_EXCEPTION(
                BCOS_ERROR(StorageError::UnknownEntryType, "Import more than 1 value"));
        }

        auto& value = *values.begin();
        m_value = std::move(value);
        m_size = 0;

        m_dirty = true;
    }

    auto&& exportFields()
    {
        m_size = 0;
        return std::move(m_value);
    }

    bool valid() const noexcept
    {
        return ((m_status != Status::DELETED) && (m_status != Status::PURGED));
    }

private:
    std::string_view valueView(const ValueType& value) const
    {
        std::string_view view;
        std::visit([this, &view](auto&& valueInside) { view = valueView(valueInside); }, value);
        return view;
    }

    template <typename T>
    std::string_view valueView(const T& value) const
    {
        std::string_view view((const char*)value.data(), value.size());
        return view;
    }

    template <typename T>
    std::string_view valueView(const std::shared_ptr<T>& value) const
    {
        std::string_view view((const char*)value->data(), value->size());
        return view;
    }

    ValueType m_value;                 // should serialization
    int32_t m_size = 0;                // no need to serialization
    Status m_status = Status::NORMAL;  // should serialization
    bool m_dirty = false;              // no need to serialization
};
}  // namespace bcos::storage
