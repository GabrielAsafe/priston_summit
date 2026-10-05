#include "systems/social/clan/v2/legacy/LegacyClanMembershipAdapter.h"

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"

namespace gameplay
{
namespace social
{
namespace clan
{
namespace legacy
{
LegacyClanMembershipAdapter::LegacyClanMembershipAdapter(
    const player::legacy::LegacyOnlinePlayerRegistry& players)
    : players_(players)
{
}

ClanMembershipMutationResult LegacyClanMembershipAdapter::SynchronizeMark(
    const ClanMembershipMutationRequest& request)
{
    ClanMembershipMutationResult result;
    result.player = request.player;
    result.mark = request.mark;

    rsPLAYINFO* player = players_.Resolve(request.player);
    if(!player)
    {
        result.code = ClanMembershipMutationCode::PlayerNotFound;
        return result;
    }

    player->smCharInfo.ClassClan = static_cast<int>(request.mark);
    player->dwClanCode = request.mark > 0
        ? GetClanCode(static_cast<int>(request.mark))
        : 0u;
    result.code = ClanMembershipMutationCode::Applied;
    return result;
}

LegacyClanMembershipAdapter& ServerLegacyClanMembershipAdapter()
{
    static LegacyClanMembershipAdapter adapter(
        player::legacy::ServerLegacyOnlinePlayerRegistry());
    return adapter;
}
}
}
}
}
