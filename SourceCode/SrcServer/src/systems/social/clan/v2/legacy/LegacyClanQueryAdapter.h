#pragma once

#include "gameplay/social/clan/v2/ClanQuery.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

namespace gameplay
{
namespace social
{
namespace clan
{
namespace legacy
{
class LegacyClanQueryAdapter final : public IClanQuery
{
public:
    explicit LegacyClanQueryAdapter(const player::legacy::LegacyOnlinePlayerRegistry& players);
    bool TryGetMembership(PlayerId player, ClanMembershipSnapshot& out) const override;

private:
    const player::legacy::LegacyOnlinePlayerRegistry& players_;
};

const LegacyClanQueryAdapter& ServerLegacyClanQueryAdapter();
}
}
}
}
