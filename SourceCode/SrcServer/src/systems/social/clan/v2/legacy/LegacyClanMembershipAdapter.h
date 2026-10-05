#pragma once

#include "gameplay/social/clan/v2/ClanMembershipService.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

namespace gameplay
{
namespace social
{
namespace clan
{
namespace legacy
{
class LegacyClanMembershipAdapter final : public IClanMembershipMutationPort
{
public:
    explicit LegacyClanMembershipAdapter(
        const player::legacy::LegacyOnlinePlayerRegistry& players);

    ClanMembershipMutationResult SynchronizeMark(
        const ClanMembershipMutationRequest& request) override;

private:
    const player::legacy::LegacyOnlinePlayerRegistry& players_;
};

LegacyClanMembershipAdapter& ServerLegacyClanMembershipAdapter();
}
}
}
}
