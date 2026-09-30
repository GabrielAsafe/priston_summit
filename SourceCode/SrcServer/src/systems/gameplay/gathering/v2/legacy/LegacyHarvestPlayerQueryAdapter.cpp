#include "systems/gameplay/gathering/v2/legacy/LegacyHarvestPlayerQueryAdapter.h"

#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/world/v2/legacy/LegacyWorldAdapter.h"
#include "core/server/legacy_runtime/onserver.h"

namespace gameplay
{
namespace gathering
{
namespace legacy
{
LegacyHarvestPlayerQueryAdapter::LegacyHarvestPlayerQueryAdapter(
    const player::legacy::LegacyOnlinePlayerRegistry& players)
    : players_(players)
{
}

bool LegacyHarvestPlayerQueryAdapter::TryGet(PlayerId playerId, HarvestPlayerSnapshot& out) const
{
    rsPLAYINFO* player = players_.Resolve(playerId);
    if (!player || !player->getName())
        return false;

    WorldPosition position;
    if (!world::legacy::ServerLegacyWorldAdapter().TryGetPosition(EntityId(playerId.value), position))
        return false;

    HarvestPlayerSnapshot snapshot;
    snapshot.player = playerId;
    snapshot.playerName = player->getName();
    snapshot.area = static_cast<int>(position.map.value);
    snapshot.x = WorldFixedToGrid(position.x);
    snapshot.y = WorldFixedToGrid(position.y);
    snapshot.z = WorldFixedToGrid(position.z);
    out = snapshot;
    return true;
}

const LegacyHarvestPlayerQueryAdapter& ServerLegacyHarvestPlayerQueryAdapter()
{
    static LegacyHarvestPlayerQueryAdapter adapter(player::legacy::ServerLegacyOnlinePlayerRegistry());
    return adapter;
}
}
}
}
