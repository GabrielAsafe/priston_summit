#pragma once

#include <cstdint>

namespace gameplay
{
namespace inventory
{
struct InventoryUseDelayState
{
    std::uint32_t elapsedTicks;
    bool active;

    InventoryUseDelayState()
        : elapsedTicks(0), active(false) {}
};

class InventoryUseDelayPolicy
{
public:
    static InventoryUseDelayState AdvancePotion(std::uint32_t elapsedTicks)
    {
        return Advance(elapsedTicks, 50u);
    }

    static InventoryUseDelayState AdvanceCrystal(std::uint32_t elapsedTicks)
    {
        return Advance(elapsedTicks, 70u);
    }

private:
    static InventoryUseDelayState Advance(std::uint32_t elapsedTicks,
                                          std::uint32_t expirationTick)
    {
        InventoryUseDelayState result;
        result.active = true;
        result.elapsedTicks = elapsedTicks + 1u;
        if(result.elapsedTicks > expirationTick)
        {
            result.elapsedTicks = 0u;
            result.active = false;
        }
        return result;
    }
};
}
}
