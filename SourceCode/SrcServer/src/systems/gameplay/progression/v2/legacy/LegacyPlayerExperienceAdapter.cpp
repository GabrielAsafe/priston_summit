#include "systems/gameplay/progression/v2/legacy/LegacyPlayerExperienceAdapter.h"

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"

namespace gameplay
{
namespace progression
{
namespace legacy
{
LegacyPlayerExperienceAdapter::LegacyPlayerExperienceAdapter(
    const player::legacy::LegacyOnlinePlayerRegistry& players)
    : players_(players)
{
}

PlayerExperienceGrantResult LegacyPlayerExperienceAdapter::Grant(
    const PlayerExperienceGrantRequest& request)
{
    PlayerExperienceGrantResult result;
    result.player = request.player;
    result.amount = request.amount;

    rsPLAYINFO* player = players_.Resolve(request.player);
    if(!player)
    {
        result.code = PlayerExperienceGrantCode::PlayerNotFound;
        return result;
    }

    player->AddExp(
        static_cast<INT64>(request.amount),
        static_cast<DWORD>(request.sourceEntity.value),
        request.party);
    result.code = PlayerExperienceGrantCode::Granted;
    return result;
}

LegacyPlayerExperienceAdapter& ServerLegacyPlayerExperienceAdapter()
{
    static LegacyPlayerExperienceAdapter adapter(player::legacy::ServerLegacyOnlinePlayerRegistry());
    return adapter;
}
}
}
}
