#include "systems/gameplay/world/v2/legacy/LegacyStageAdapter.h"

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"

namespace gameplay
{
namespace world
{
namespace legacy
{
namespace
{
bool ValidMap(MapId map)
{
    return StageArea && StageAreaCount > 0 && map.Valid() && map.value < static_cast<std::uint32_t>(StageAreaCount);
}
}

bool LegacyStageAdapter::TryGetStage(MapId map, StageSnapshot& out) const
{
    if (!ValidMap(map) || !StageArea)
        return false;
    out = StageSnapshot(map, StageArea[map.value].lpStage != 0);
    return true;
}

bool LegacyStageAdapter::IsReady(MapId map) const
{
    StageSnapshot stage;
    return TryGetStage(map, stage) && stage.ready;
}

bool LegacyStageAdapter::TryGetGroundHeight(MapId map,
                                            std::int32_t x,
                                            std::int32_t z,
                                            std::int32_t& y) const
{
    if (!ValidMap(map) || !StageArea || !StageArea[map.value].lpStage)
        return false;
    const int height = StageArea[map.value].lpStage->GetHeight(x, z);
    if (height <= 0)
        return false;
    y = height;
    return true;
}

const LegacyStageAdapter& ServerLegacyStageAdapter()
{
    static LegacyStageAdapter adapter;
    return adapter;
}
}
}
}
