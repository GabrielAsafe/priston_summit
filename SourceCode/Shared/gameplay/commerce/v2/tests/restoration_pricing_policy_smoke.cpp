#include "Shared/gameplay/commerce/v2/RestorationPricingPolicy.h"

#include <cassert>
#include <iostream>

int main()
{
    using gameplay::commerce::RestorationPricingPolicy;

    assert(RestorationPricingPolicy::QuotedPrice(1) == 0);
    assert(RestorationPricingPolicy::QuotedPrice(7) == 0);
    assert(RestorationPricingPolicy::QuotedPrice(8) == RestorationPricingPolicy::UnsupportedPrice);
    assert(RestorationPricingPolicy::QuotedPrice(9) == 6000);
    assert(RestorationPricingPolicy::QuotedPrice(21) == 18000);
    assert(RestorationPricingPolicy::QuotedPrice(22) == 1300);
    assert(RestorationPricingPolicy::QuotedPrice(26) == 1300);

    assert(RestorationPricingPolicy::ChargePrice(1) == 100000);
    assert(RestorationPricingPolicy::ChargePrice(7) == 100000);
    assert(RestorationPricingPolicy::ChargePrice(8) == RestorationPricingPolicy::UnsupportedPrice);
    assert(RestorationPricingPolicy::ChargePrice(9) == 1000);
    assert(RestorationPricingPolicy::ChargePrice(20) == 4000); // preserves branch ordering
    assert(RestorationPricingPolicy::ChargePrice(21) == 5000);
    assert(RestorationPricingPolicy::ChargePrice(26) == 5000);

    // Explicitly pin the current legacy discrepancy; migration must not silently
    // turn a refactor into an economy rebalance.
    assert(RestorationPricingPolicy::QuotedPrice(9) != RestorationPricingPolicy::ChargePrice(9));

    std::cout << "restoration pricing policy smoke: PASS\n";
    return 0;
}
