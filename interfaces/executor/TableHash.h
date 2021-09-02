#pragma once

#include "../../libutilities/Common.h"
#include <stddef.h>
#include <string>
#include <string_view>

namespace bcos
{
namespace executor
{
class TableHash
{
public:
    using Ptr = std::shared_ptr<TableHash>;
    using ConstPtr = std::shared_ptr<const TableHash>;

    virtual std::string_view name() const = 0;
    virtual void setName(std::string name) = 0;

    virtual crypto::HashType hash() const = 0;
    virtual void setHash(crypto::HashType hash) = 0;
};

class TableDBHashFactory
{
public:
    using Ptr = std::shared_ptr<TableDBHashFactory>;
    using ConstPtr = std::shared_ptr<const TableDBHashFactory>;

    virtual ~TableDBHashFactory() {};

    virtual TableDBHashFactory::Ptr createContractStatus() = 0;
};
}  // namespace executor
}  // namespace bcos
