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
#include "boost/algorithm/string.hpp"
#include "tbb/spin_mutex.h"
#include "tbb/spin_rw_mutex.h"
#include "tbb/tbb_thread.h"
#include <boost/throw_exception.hpp>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#define STORAGE_LOG(LEVEL) BCOS_LOG(LEVEL) << "[STORAGE]"

namespace bcos
{
namespace storage
{
struct Condition : public std::enable_shared_from_this<Condition>
{
    Condition() = default;
    virtual ~Condition() = default;
    virtual void NE(const std::string& value) { m_conditions.emplace_back(Comparator::NE, value); }
    // string compare, "2" > "12"
    virtual void GT(const std::string& value) { m_conditions.emplace_back(Comparator::GT, value); }
    virtual void GE(const std::string& value) { m_conditions.emplace_back(Comparator::GE, value); }
    // string compare, "12" < "2"
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
    using ConstPtr = std::shared_ptr<const TableInfo>;

    explicit TableInfo(const std::string& _tableName, const std::string_view& _key,
        const std::string_view& _fields)
      : name(_tableName), key(_key)
    {
        boost::split(fields, _fields, boost::is_any_of(","));
        generateFieldsIndex();
    }
    explicit TableInfo(const std::string& _tableName, const std::string& _key,
        const std::vector<std::string>& _fields)
      : name(_tableName), key(_key), fields(_fields)
    {
        generateFieldsIndex();
    }

    void generateFieldsIndex()
    {
        size_t i = 0;

        for (auto& field : fields)
        {
            auto [it, success] = field2Index.emplace(field, i++);
            (void)it;
            if (!success)
            {
                BOOST_THROW_EXCEPTION(bcos::Exception("Field exists! " + field));
            }
        }
    }

    std::string name;
    std::string key;
    std::vector<std::string> fields;
    std::map<std::string, size_t, std::less<>> field2Index;
};

}  // namespace storage
}  // namespace bcos
