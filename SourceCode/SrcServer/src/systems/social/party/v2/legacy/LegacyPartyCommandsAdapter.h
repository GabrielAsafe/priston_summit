#pragma once

#include "gameplay/social/party/v2/PartyCommands.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

namespace gameplay
{
namespace social
{
namespace party
{
namespace legacy
{
class LegacyPartyCommandsAdapter final : public IPartyCommands
{
public:
    explicit LegacyPartyCommandsAdapter(
        const player::legacy::LegacyOnlinePlayerRegistry& players);

    bool Join(PlayerId requester, PlayerId target) const override;
    bool TransferLeadership(PlayerId requester, PlayerId target) const override;
    bool Release(PlayerId requester) const override;
    bool Kick(PlayerId requester, PlayerId target) const override;
    bool Leave(PlayerId requester) const override;
    bool ClearMembership(PlayerId member) const override;
    bool SynchronizeMembership(PlayerId receiver, const PartyMembershipSync& sync) const override;

private:
    const player::legacy::LegacyOnlinePlayerRegistry& players_;
};

const LegacyPartyCommandsAdapter& ServerLegacyPartyCommandsAdapter();
}
}
}
}
