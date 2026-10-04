#include "systems/gameplay/world/v2/legacy/LegacyWorldAdapter.h"
#include "systems/gameplay/world/v2/legacy/LegacyStageAdapter.h"

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "systems/world/field/field.h"
#include "core/server/legacy_runtime/onserver.h"

namespace gameplay
{
namespace world
{
namespace legacy
{
LegacyWorldAdapter::LegacyWorldAdapter(const player::legacy::LegacyOnlinePlayerRegistry& players, const IStageQuery& stages)
    : players_(players), stages_(stages)
{
}

bool LegacyWorldAdapter::TryGetPosition(EntityId entity, WorldPosition& out) const
{
    if (!entity.Valid())
        return false;

    // Player object serials share the entity serial namespace. Resolve the
    // server-authoritative area first because smCHAR::OnStageField is not a
    // reliable map identity for all server-side player/companion cases.
    rsPLAYINFO* player = players_.Resolve(PlayerId(entity.value));
    if (player && player->Position.Area >= 0 && player->Position.Area < StageAreaCount)
    {
        out = WorldPosition(MapId(static_cast<std::uint32_t>(player->Position.Area)),
                            player->Position.x, player->Position.y, player->Position.z);
        return true;
    }

    smCHAR* character = srFindCharFromSerial(static_cast<DWORD>(entity.value));
    if (!character || character->OnStageField < 0 || character->OnStageField >= 2 ||
        !StageField[character->OnStageField])
        return false;

    const int fieldCode = StageField[character->OnStageField]->FieldCode;
    if (fieldCode < 0 || fieldCode >= StageAreaCount)
        return false;
    out = WorldPosition(MapId(static_cast<std::uint32_t>(fieldCode)),
                        character->pX, character->pY, character->pZ);
    return true;
}

bool LegacyWorldAdapter::TryGetGroundHeight(MapId map,
                                            std::int32_t x,
                                            std::int32_t z,
                                            std::int32_t& y) const
{
    return stages_.TryGetGroundHeight(map, x, z, y);
}

bool LegacyWorldAdapter::IsValidPosition(const WorldPosition& position) const
{
    std::int32_t ground = 0;
    return TryGetGroundHeight(position.map, position.x, position.z, ground);
}

bool LegacyWorldAdapter::IsSameArea(EntityId lhs, EntityId rhs) const
{
    WorldPosition a;
    WorldPosition b;
    return TryGetPosition(lhs, a) && TryGetPosition(rhs, b) && a.map == b.map;
}

const LegacyWorldAdapter& ServerLegacyWorldAdapter()
{
    static LegacyWorldAdapter adapter(player::legacy::ServerLegacyOnlinePlayerRegistry(), ServerLegacyStageAdapter());
    return adapter;
}
}
}
}
