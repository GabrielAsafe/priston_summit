#pragma once

#include <vector>

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace social
{
namespace party
{
struct PartyMemberSnapshot
{
    PlayerId player;
    MapId map;
    bool online;

    PartyMemberSnapshot() : player(), map(), online(false) {}
    PartyMemberSnapshot(PlayerId playerValue, MapId mapValue, bool onlineValue)
        : player(playerValue), map(mapValue), online(onlineValue) {}
};

struct PartySnapshot
{
    PlayerId leader;
    std::vector<PartyMemberSnapshot> members;

    PartySnapshot() : leader(), members() {}
};

class IPartyQuery
{
public:
    virtual ~IPartyQuery() {}
    virtual bool TryGetParty(PlayerId member, PartySnapshot& out) const = 0;
};
}
}
}
