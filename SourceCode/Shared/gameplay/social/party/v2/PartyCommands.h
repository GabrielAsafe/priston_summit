#pragma once

#include <vector>

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace social
{
namespace party
{
struct PartyMembershipSync
{
    std::vector<PlayerId> members;
};

class IPartyCommands
{
public:
    virtual ~IPartyCommands() {}

    // Preserves the legacy PARTY_JOIN ingress semantics: join the target's
    // party, invite the target into the requester's party, or create a new
    // party when both players are currently ungrouped.
    virtual bool Join(PlayerId requester, PlayerId target) const = 0;
    virtual bool TransferLeadership(PlayerId requester, PlayerId target) const = 0;
    virtual bool Release(PlayerId requester) const = 0;
    virtual bool Kick(PlayerId requester, PlayerId target) const = 0;
    virtual bool Leave(PlayerId requester) const = 0;
    virtual bool ClearMembership(PlayerId member) const = 0;
    virtual bool SynchronizeMembership(PlayerId receiver, const PartyMembershipSync& sync) const = 0;
};
}
}
}
