#include "systems/gameplay/loot/v2/legacy/LegacyLootAdapter.h"

#include <cstddef>

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"

namespace gameplay
{
namespace loot
{
namespace legacy
{
LegacyLootAdapter::LegacyLootAdapter(const player::legacy::LegacyOnlinePlayerRegistry& players)
    : players_(players)
{
}

LootDropHandle LegacyLootAdapter::MakeHandle(const STG_AREA* area, const STG_ITEMS* item)
{
    if(!StageArea || !area || !item || !item->state)
        return LootDropHandle();

    const std::ptrdiff_t areaIndex = area - StageArea;
    if(areaIndex < 0 || areaIndex >= STAGE_AREA_MAX)
        return LootDropHandle();

    const std::ptrdiff_t slotIndex = item - area->StgItems;
    if(slotIndex < 0 || slotIndex >= STG_ITEM_MAX)
        return LootDropHandle();

    return LootDropHandle(
        static_cast<std::uint32_t>(areaIndex),
        static_cast<std::uint32_t>(slotIndex),
        static_cast<std::uint32_t>(item->dwCreateTime),
        ItemId(static_cast<std::uint32_t>(item->Item.ItemInfo.CODE)),
        static_cast<std::uint32_t>(item->Item.ItemInfo.ItemHeader.dwChkSum));
}

LootPickupResult LegacyLootAdapter::Pickup(PlayerId player, LootDropHandle drop)
{
    LootPickupResult result;
    result.player = player;
    result.drop = drop;

    if(!drop.Valid() || drop.areaIndex >= STAGE_AREA_MAX || drop.slotIndex >= STG_ITEM_MAX || !StageArea)
    {
        result.code = LootPickupCode::InvalidDrop;
        return result;
    }

    rsPLAYINFO* legacyPlayer = players_.Resolve(player);
    if(!legacyPlayer)
    {
        result.code = LootPickupCode::PlayerNotFound;
        return result;
    }

    STG_AREA* area = &StageArea[drop.areaIndex];
    STG_ITEMS* item = &area->StgItems[drop.slotIndex];
    if(!item->state)
    {
        result.code = LootPickupCode::DropNotFound;
        return result;
    }

    if(static_cast<std::uint32_t>(item->dwCreateTime) != drop.createdAt ||
       static_cast<std::uint32_t>(item->Item.ItemInfo.CODE) != drop.item.value ||
       static_cast<std::uint32_t>(item->Item.ItemInfo.ItemHeader.dwChkSum) != drop.checksum)
    {
        result.code = LootPickupCode::StaleDrop;
        return result;
    }

    result.code = rsCompanionPickupStageItem(legacyPlayer, area, item)
                      ? LootPickupCode::PickedUp
                      : LootPickupCode::LegacyRejected;
    return result;
}

LegacyLootAdapter& ServerLegacyLootAdapter()
{
    static LegacyLootAdapter adapter(player::legacy::ServerLegacyOnlinePlayerRegistry());
    return adapter;
}
}
}
}
