#pragma once

#include "gameplay/loot/v2/LootService.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

class STG_AREA;
struct STG_ITEMS;

namespace gameplay
{
namespace loot
{
namespace legacy
{
class LegacyLootAdapter final : public ILootService
{
public:
    explicit LegacyLootAdapter(const player::legacy::LegacyOnlinePlayerRegistry& players);

    LootPickupResult Pickup(PlayerId player, LootDropHandle drop) override;

    static LootDropHandle MakeHandle(const STG_AREA* area, const STG_ITEMS* item);

private:
    const player::legacy::LegacyOnlinePlayerRegistry& players_;
};

LegacyLootAdapter& ServerLegacyLootAdapter();
}
}
}
