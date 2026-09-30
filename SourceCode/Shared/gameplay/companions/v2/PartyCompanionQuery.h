#pragma once

#include <cstdint>

#include "gameplay/companions/v2/CompanionQuery.h"
#include "gameplay/social/party/v2/PartyQuery.h"

namespace gameplay
{
namespace companions
{
inline std::uint32_t ActivePartyCompanionsOnMap(
    const social::party::IPartyQuery& partyQuery,
    const ICompanionQuery& companionQuery,
    PlayerId member,
    MapId map)
{
    social::party::PartySnapshot party;
    if(!partyQuery.TryGetParty(member, party))
        return 0u;

    std::uint32_t total = 0u;
    for(std::vector<social::party::PartyMemberSnapshot>::const_iterator it = party.members.begin();
        it != party.members.end(); ++it)
    {
        if(!it->online || it->map != map)
            continue;
        total += companionQuery.ActiveCount(it->player);
    }
    return total;
}
}
}
