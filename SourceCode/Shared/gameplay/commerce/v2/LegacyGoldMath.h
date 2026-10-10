#pragma once
// PATCH 231: explicit, overflow-safe arithmetic at the legacy 32-bit wallet boundary.
// Does not raise the persisted/protocol gold cap or trust client balances.
#include <cstdint>

namespace gameplay { namespace commerce {
static const std::int64_t kLegacyGoldLimit = 1000000000LL;
inline bool TryLegacyGoldCredit(int before, int amount, int& next)
{
    const std::int64_t candidate = static_cast<std::int64_t>(before) + amount;
    if(before < 0 || before > kLegacyGoldLimit || amount < 0 ||
       candidate > kLegacyGoldLimit) return false;
    next = static_cast<int>(candidate);
    return true;
}
inline bool TryLegacyGoldDebit(int before, int amount, int& next)
{
    const std::int64_t candidate = static_cast<std::int64_t>(before) - amount;
    if(before < 0 || before > kLegacyGoldLimit || amount < 0 || candidate < 0)
        return false;
    next = static_cast<int>(candidate);
    return true;
}
inline bool TryLegacyGoldSet(int amount, int& next)
{
    if(amount < 0 || amount > kLegacyGoldLimit) return false;
    next = amount;
    return true;
}
}} // gameplay::commerce
