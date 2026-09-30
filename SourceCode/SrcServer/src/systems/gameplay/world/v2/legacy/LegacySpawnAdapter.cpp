#include "systems/gameplay/world/v2/legacy/LegacySpawnAdapter.h"

#include <vector>

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"

extern smCHAR* OpenMonsterFromName(char* szName, int x, int y, int z, rsPLAYINFO* lpPlayInfo);

namespace gameplay
{
namespace world
{
namespace legacy
{
LegacySpawnAdapter::LegacySpawnAdapter(const player::legacy::LegacyOnlinePlayerRegistry& players)
    : players_(players)
{
}

SpawnResult LegacySpawnAdapter::Spawn(const SpawnRequest& request)
{
    if (!request.Valid())
        return SpawnResult(SpawnStatus::InvalidRequest);

    rsPLAYINFO* owner = 0;
    if (request.owner.Valid())
    {
        owner = players_.Resolve(request.owner);
        if (!owner)
            return SpawnResult(SpawnStatus::OwnerUnavailable);
    }

    // OpenMonsterFromName does not mutate its name argument, but its historical
    // signature predates const-correctness. Give it a writable, request-local copy.
    std::vector<char> name(request.templateName.begin(), request.templateName.end());
    name.push_back('\0');

    smCHAR* spawned = OpenMonsterFromName(
        name.data(), request.position.x, request.position.y, request.position.z, owner);
    if (!spawned || spawned->dwObjectSerial == 0u)
        return SpawnResult(SpawnStatus::NotSpawned);

    return SpawnResult(SpawnStatus::Spawned, EntityId(static_cast<std::uint32_t>(spawned->dwObjectSerial)));
}

LegacySpawnAdapter& ServerLegacySpawnAdapter()
{
    static LegacySpawnAdapter adapter(player::legacy::ServerLegacyOnlinePlayerRegistry());
    return adapter;
}
}
}
}
