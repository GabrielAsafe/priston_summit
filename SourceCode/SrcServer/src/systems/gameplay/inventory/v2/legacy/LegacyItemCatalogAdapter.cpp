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

LegacyItemCatalogAdapter& ServerLegacyItemCatalogAdapter()
{
    static LegacyItemCatalogAdapter adapter;
    return adapter;
}
}
}
}
