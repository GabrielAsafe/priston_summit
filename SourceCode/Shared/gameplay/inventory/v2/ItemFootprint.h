#pragma once
// The inventory V2 contract for EVERY inventory item, not only equipment.
// The persistence layer knows DAT bytes; only catalog adapters know legacy item tables.
#include <cstdint>
#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay { namespace inventory {
struct ItemFootprint {
    ItemId item;
    std::uint16_t widthPixels;
    std::uint16_t heightPixels;
    ItemFootprint() : item(), widthPixels(0), heightPixels(0) {}
    bool ValidFor(ItemId wanted) const {
        return wanted.Valid() && item.Valid() && item.value == wanted.value &&
               widthPixels > 0 && heightPixels > 0;
    }
};
class IItemFootprintCatalog {
public:
    virtual ~IItemFootprintCatalog() {}
    virtual bool TryGetFootprint(ItemId item, ItemFootprint& out) const = 0;
};
}} // namespace gameplay::inventory
