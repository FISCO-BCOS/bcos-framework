#pragma once

#include "libutilities/Common.h"
#include <stddef.h>
#include <string>
#include <string_view>

namespace bcos
{
namespace executor
{
class ContractStatus
{
public:
    using Ptr = std::shared_ptr<ContractStatus>;
    using ConstPtr = std::shared_ptr<const ContractStatus>;

    virtual std::string_view contract() const = 0;
    virtual void setContract(const std::string_view& contract) = 0;
    virtual void setContract(std::string&& contract) = 0;

    virtual bcos::bytesConstRef dbHash() const = 0;
    virtual void setDBHash(const bcos::bytesConstRef& dbHash) = 0;
    virtual void setDBHash(bcos::bytes&& dbHash) = 0;
};
}  // namespace executor
}  // namespace bcos