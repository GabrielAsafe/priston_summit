#pragma once

#include <cstdint>
#include <vector>

namespace gameplay
{
namespace inventory
{
struct InventoryIntegritySlot
{
    std::uint32_t index;
    bool active;
    bool potion;
    std::int32_t y;
    std::int32_t itemClass;
    std::uint32_t checksum;
    std::uint32_t head;

    InventoryIntegritySlot()
        : index(0u), active(false), potion(false), y(0), itemClass(0), checksum(0u), head(0u) {}
};

struct InventoryIntegritySnapshot
{
    std::vector<InventoryIntegritySlot> inventory;
    std::vector<InventoryIntegritySlot> temporary;
};

class InventoryIntegrityService
{
public:
    static std::uint32_t ComputeFingerprint(const InventoryIntegritySnapshot& snapshot)
    {
        std::uint32_t fingerprint = 0u;
        Accumulate(snapshot.inventory, fingerprint);
        Accumulate(snapshot.temporary, fingerprint);
        return fingerprint;
    }

private:
    static void Accumulate(const std::vector<InventoryIntegritySlot>& slots,
                           std::uint32_t& fingerprint)
    {
        for(std::vector<InventoryIntegritySlot>::const_iterator it = slots.begin();
            it != slots.end(); ++it)
        {
            if(!it->active || it->potion)
                continue;

            const std::uint32_t ordinal = it->index + 1u;
            fingerprint += ordinal * static_cast<std::uint32_t>(it->y);
            fingerprint += ordinal * static_cast<std::uint32_t>(it->itemClass);
            fingerprint += it->checksum;
            fingerprint += it->head;
        }
    }
};
}
}
