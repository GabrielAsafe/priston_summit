#include "gameplay/inventory/v2/InventoryIntegrityService.h"

#include <cassert>
#include <cstdint>

namespace
{
gameplay::inventory::InventoryIntegritySlot Slot(std::uint32_t index,
                                                  bool active,
                                                  bool potion,
                                                  int y,
                                                  int itemClass,
                                                  std::uint32_t checksum,
                                                  std::uint32_t head)
{
    gameplay::inventory::InventoryIntegritySlot slot;
    slot.index = index;
    slot.active = active;
    slot.potion = potion;
    slot.y = y;
    slot.itemClass = itemClass;
    slot.checksum = checksum;
    slot.head = head;
    return slot;
}
}

int main()
{
    using namespace gameplay::inventory;

    InventoryIntegritySnapshot snapshot;
    snapshot.inventory.push_back(Slot(0u, true, false, 10, 20, 30u, 40u));
    snapshot.inventory.push_back(Slot(1u, false, false, 99, 99, 99u, 99u));
    snapshot.inventory.push_back(Slot(2u, true, true, 77, 77, 77u, 77u));
    snapshot.temporary.push_back(Slot(3u, true, false, 5, 6, 7u, 8u));

    const std::uint32_t expected =
        1u * 10u + 1u * 20u + 30u + 40u +
        4u * 5u + 4u * 6u + 7u + 8u;
    assert(InventoryIntegrityService::ComputeFingerprint(snapshot) == expected);

    // Preserve DWORD-style modulo arithmetic for negative legacy coordinates/classes.
    InventoryIntegritySnapshot wrapping;
    wrapping.inventory.push_back(Slot(1u, true, false, -1, -2, 0u, 0u));
    const std::uint32_t wrapExpected =
        2u * static_cast<std::uint32_t>(-1) +
        2u * static_cast<std::uint32_t>(-2);
    assert(InventoryIntegrityService::ComputeFingerprint(wrapping) == wrapExpected);

    InventoryIntegritySnapshot empty;
    assert(InventoryIntegrityService::ComputeFingerprint(empty) == 0u);
    return 0;
}
