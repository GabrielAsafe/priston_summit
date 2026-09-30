#include "systems/social/clan/v2/legacy/LegacyClanQueryAdapter.h"

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
LegacyClanQueryAdapter::LegacyClanQueryAdapter(const player::legacy::LegacyOnlinePlayerRegistry& players)
    : players_(players)
{
}

bool LegacyClanQueryAdapter::TryGetMembership(PlayerId playerId, ClanMembershipSnapshot& out) const
{
    rsPLAYINFO* player = players_.Resolve(playerId);
    if(!player)
        return false;

    ClanMembershipSnapshot snapshot;
    snapshot.player = playerId;
    snapshot.clan = ClanId(static_cast<std::uint32_t>(player->dwClanCode));
    snapshot.mark = static_cast<std::int32_t>(player->smCharInfo.ClassClan);
    out = snapshot;
    return true;
}

const LegacyClanQueryAdapter& ServerLegacyClanQueryAdapter()
{
    static LegacyClanQueryAdapter adapter(player::legacy::ServerLegacyOnlinePlayerRegistry());
    return adapter;
}
}
}
}
}
