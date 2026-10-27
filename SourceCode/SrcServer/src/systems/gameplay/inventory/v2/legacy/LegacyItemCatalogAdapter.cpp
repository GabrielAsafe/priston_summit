#include "systems/gameplay/inventory/v2/legacy/LegacyItemCatalogAdapter.h"

#include "systems/inventory/legacy/sinItem.h"
#include "core/data/legacy_loader/fileread.h"

extern int DefaultItemCount;
extern sDEF_ITEMINFO DefaultItems[];

#include <cctype>
#include <cstring>

namespace gameplay
{
namespace inventory
{
namespace legacy
{
namespace
{
// Item metadata is a V2, data-only projection of verified historical client
// definitions. It must not depend on legacy server sItem: that array is filled
// from a code-only MinGW map (CODE/LastCategory), without Class or w/h.
struct Metadata317 {
    std::uint32_t code;
    std::uint16_t widthPixels;
    std::uint16_t heightPixels;
    std::uint32_t classBits;
};
static const Metadata317 kMetadata317[] = {
#include "systems/gameplay/inventory/v2/content/RecoveredItemMetadata317.inc"
};
static const Metadata317* FindMetadata317(ItemId item)
{
    if(!item.Valid()) return nullptr;
    // Generated table sorted by numeric code; binary search never guesses.
    std::size_t lo=0, hi=sizeof(kMetadata317)/sizeof(kMetadata317[0]);
    while(lo<hi) {
        const std::size_t mid=lo+(hi-lo)/2;
        if(kMetadata317[mid].code<item.value) lo=mid+1;
        else hi=mid;
    }
    if(lo>=sizeof(kMetadata317)/sizeof(kMetadata317[0]) ||
       kMetadata317[lo].code!=item.value) return nullptr;
    return &kMetadata317[lo];
}

ItemCategory TranslateCategory(const sITEM& item)
{
    if(item.Class == ITEM_CLASS_POTION)
        return ItemCategory::Consumable;
    if(item.Class != 0 && item.Class != INVENTORY_POS_BOX)
        return ItemCategory::Equipment;
    return ItemCategory::General;
}

bool EqualsIgnoreCase(const char* left, const std::string& right)
{
    if(!left)
        return false;
    const std::size_t length = std::strlen(left);
    if(length != right.size())
        return false;
    for(std::size_t index = 0; index < length; ++index)
    {
        const unsigned char lhs = static_cast<unsigned char>(left[index]);
        const unsigned char rhs = static_cast<unsigned char>(right[index]);
        if(std::tolower(lhs) != std::tolower(rhs))
            return false;
    }
    return true;
}

bool IsDefaultGrantable(std::uint32_t code)
{
    for(int index = 0; index < DefaultItemCount; ++index)
    {
        if(static_cast<std::uint32_t>(DefaultItems[index].Item.CODE) == code &&
           !DefaultItems[index].Item.UniqueItem)
            return true;
    }
    return false;
}

std::string ResolveDisplayName(std::uint32_t code)
{
    for(int index = 0; index < DefaultItemCount; ++index)
    {
        if(static_cast<std::uint32_t>(DefaultItems[index].Item.CODE) == code)
            return std::string(DefaultItems[index].Item.ItemName);
    }
    return std::string();
}
}

bool LegacyItemCatalogAdapter::TryGetDefinition(ItemId item, ItemDefinition& out) const
{
    if(!item.Valid())
        return false;

    for(int index = 0; index < MAX_ITEM; ++index)
    {
        const sITEM& legacyItem = sItem[index];
        if(legacyItem.CODE == 0)
            continue;
        if(static_cast<std::uint32_t>(legacyItem.CODE) != item.value)
            continue;

        const ItemCategory category = TranslateCategory(legacyItem);
        out = ItemDefinition(
            item,
            std::string(legacyItem.LastCategory),
            category,
            category == ItemCategory::Consumable,
            IsDefaultGrantable(item.value),
            ResolveDisplayName(item.value));
        return true;
    }

    return false;
}

bool LegacyItemCatalogAdapter::TryGetDefinitionByContentKey(
    const std::string& contentKey,
    ItemDefinition& out) const
{
    if(contentKey.empty())
        return false;

    for(int index = 0; index < MAX_ITEM; ++index)
    {
        const sITEM& legacyItem = sItem[index];
        if(!legacyItem.CODE || !EqualsIgnoreCase(legacyItem.LastCategory, contentKey))
            continue;
        return TryGetDefinition(
            ItemId(static_cast<std::uint32_t>(legacyItem.CODE)), out);
    }
    return false;
}


// This is the ONLY translation of old sItem equipment bits to the V2
// equipment contract. Gameplay and persistence code must use EquipmentPolicy.
// The V2 policy is authoritative for allowed slots and requirements.
// The adapter supplies verified historical class metadata, never sItem.Class.
bool LegacyItemCatalogAdapter::TryGetEquipment(ItemId item, EquipmentDefinition& out) const
{
    const Metadata317* source=FindMetadata317(item);
    if(!source) return false;
    static const struct { int slot; unsigned int classBit; } slotMap[] = {
        {3, INVENTORY_POS_ARMOR}, {4, INVENTORY_POS_AMULET},
        {5, INVENTORY_POS_RRING}, {6, INVENTORY_POS_LRING},
        {7, INVENTORY_POS_SHELTOM}, {8, INVENTORY_POS_ARMLET},
        {9, INVENTORY_POS_GLOVES}, {10, INVENTORY_POS_BOOTS},
        {14, INVENTORY_POS_COSTUME}, {15, INVENTORY_POS_EARRINGSL},
        {16, INVENTORY_POS_EARRINGSR}
    };
    EquipmentDefinition definition;
    definition.item=item;
    // Historical metadata is a data projection; hand slot eligibility must
    // not read the MinGW code-only runtime sItem.Class (which is frequently 0).
    // RIGHT=0x4, LEFT=0x2, BOTH=0x6 denotes a two-handed weapon: only the
    // right hand may select it. Pure V2 policy decides all requirements.
    const bool rightHand = (source->classBits & 0x00000004u) != 0u;
    const bool leftHand = (source->classBits & 0x00000002u) != 0u;
    definition.twoHanded = rightHand && leftHand;
    if(rightHand) definition.allowedSlots |= std::uint32_t(1) << 1;
    if(leftHand && !rightHand) definition.allowedSlots |= std::uint32_t(1) << 2;
    // A zero slot mask is meaningful: catalogued but not wearable here.
    for(const auto& entry: slotMap)
        if((source->classBits & entry.classBit)!=0u)
            definition.allowedSlots |= std::uint32_t(1) << entry.slot;
    definition.widthPixels=source->widthPixels;
    definition.heightPixels=source->heightPixels;
    out=definition;
    return true;
}

// Footprint is independent of equipment eligibility and works for potions,
// quest objects, accessories, custom historical entries, etc.
bool LegacyItemCatalogAdapter::TryGetFootprint(ItemId item, ItemFootprint& out) const
{
    out=ItemFootprint();
    const Metadata317* source=FindMetadata317(item);
    if(!source) return false; // not in verified data: no fabricated size
    out.item=item;
    out.widthPixels=source->widthPixels;
    out.heightPixels=source->heightPixels;
    return out.ValidFor(item);
}

LegacyItemCatalogAdapter& ServerLegacyItemCatalogAdapter()
{
    static LegacyItemCatalogAdapter adapter;
    return adapter;
}
}
}
}
