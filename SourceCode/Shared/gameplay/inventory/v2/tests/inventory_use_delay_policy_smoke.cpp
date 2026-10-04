#include "gameplay/inventory/v2/InventoryUseDelayPolicy.h"

#include <cassert>

int main()
{
    using gameplay::inventory::InventoryUseDelayPolicy;

    {
        const auto state = InventoryUseDelayPolicy::AdvancePotion(0u);
        assert(state.active);
        assert(state.elapsedTicks == 1u);
    }
    {
        const auto state = InventoryUseDelayPolicy::AdvancePotion(49u);
        assert(state.active);
        assert(state.elapsedTicks == 50u);
    }
    {
        const auto state = InventoryUseDelayPolicy::AdvancePotion(50u);
        assert(!state.active);
        assert(state.elapsedTicks == 0u);
    }
    {
        const auto state = InventoryUseDelayPolicy::AdvanceCrystal(69u);
        assert(state.active);
        assert(state.elapsedTicks == 70u);
    }
    {
        const auto state = InventoryUseDelayPolicy::AdvanceCrystal(70u);
        assert(!state.active);
        assert(state.elapsedTicks == 0u);
    }

    return 0;
}
