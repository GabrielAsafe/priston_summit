#pragma once

namespace gameplay::commerce
{
class RestorationPricingPolicy
{
public:
    static constexpr int UnsupportedPrice = 999999999;

    // Preserves the price currently advertised by the legacy restoration list.
    static constexpr int QuotedPrice(int agingLevel) noexcept
    {
        if (agingLevel > 0 && agingLevel < 8)
            return 0;
        if (agingLevel >= 9 && agingLevel < 12)
            return 6000;
        if (agingLevel >= 12 && agingLevel < 14)
            return 8000;
        if (agingLevel >= 14 && agingLevel < 18)
            return 18000;
        if (agingLevel >= 18 && agingLevel < 22)
            return 18000;
        if (agingLevel >= 22 && agingLevel < 27)
            return 1300;
        return UnsupportedPrice;
    }

    // Preserves the price currently charged by the legacy restore command.
    // This intentionally remains separate from QuotedPrice because the legacy
    // runtime currently exposes different values. A later product decision can
    // reconcile them without hiding a behavior change inside this migration.
    static constexpr int ChargePrice(int agingLevel) noexcept
    {
        if (agingLevel > 0 && agingLevel < 8)
            return 100000;
        if (agingLevel >= 9 && agingLevel < 12)
            return 1000;
        if (agingLevel >= 12 && agingLevel < 14)
            return 2000;
        if (agingLevel >= 14 && agingLevel < 18)
            return 3000;
        if (agingLevel >= 18 && agingLevel < 21)
            return 4000;
        if (agingLevel >= 20 && agingLevel < 27)
            return 5000;
        return UnsupportedPrice;
    }
};
}
