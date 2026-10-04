#pragma once

#include <cstdint>

namespace gameplay
{
namespace commerce
{
class PremiumPurchaseRequestSequence final
{
public:
    PremiumPurchaseRequestSequence() : next_(0u) {}

    std::uint32_t Next()
    {
        if(next_ >= 0x7FFFFFFFu)
            next_ = 1u;
        else
            ++next_;
        return next_;
    }

private:
    std::uint32_t next_;
};
}
}
