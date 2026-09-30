#include "systems/social/party/v2/legacy/LegacyPartyQueryAdapter.h"

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"

namespace gameplay
{
namespace social
{
namespace party
{
namespace legacy
{
LegacyPartyQueryAdapter::LegacyPartyQueryAdapter(
    const player::legacy::LegacyOnlinePlayerRegistry& players)
    : players_(players)
{
}

bool LegacyPartyQueryAdapter::TryGetParty(PlayerId member, PartySnapshot& out) const
{
    rsPLAYINFO* legacyMember = players_.Resolve(member);
    if(!legacyMember)
        return false;

    rsPLAYINFO* master = legacyMember->lpPartyMaster;
    if(!master && legacyMember->dwPartyInfo == PARTY_STATE_MASTER)
        master = legacyMember;
    if(!master || master->dwPartyInfo != PARTY_STATE_MASTER)
        return false;

    PartySnapshot snapshot;
    snapshot.leader = players_.IdOf(master);
    if(!snapshot.leader.Valid())
        return false;

    snapshot.members.reserve(PARTY_PLAYER_MAX);
    for(int index = 0; index < PARTY_PLAYER_MAX; ++index)
    {
        rsPLAYINFO* legacyPartyMember = master->lpPartyPlayers[index];
        const PlayerId id = players_.IdOf(legacyPartyMember);
        if(!id.Valid())
            continue;
        snapshot.members.push_back(PartyMemberSnapshot(
            id,
            MapId(static_cast<std::uint32_t>(legacyPartyMember->Position.Area)),
            true));
    }

    out = snapshot;
    return true;
}

const LegacyPartyQueryAdapter& ServerLegacyPartyQueryAdapter()
{
    static LegacyPartyQueryAdapter adapter(player::legacy::ServerLegacyOnlinePlayerRegistry());
    return adapter;
}
}
}
}
}
