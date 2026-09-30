#pragma once

#include "gameplay/social/party/v2/PartyQuery.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

namespace gameplay
{
namespace social
{
namespace party
{
namespace legacy
{
class LegacyPartyQueryAdapter final : public IPartyQuery
{
public:
    explicit LegacyPartyQueryAdapter(const player::legacy::LegacyOnlinePlayerRegistry& players);
    bool TryGetParty(PlayerId member, PartySnapshot& out) const override;

private:
    const player::legacy::LegacyOnlinePlayerRegistry& players_;
};

const LegacyPartyQueryAdapter& ServerLegacyPartyQueryAdapter();
}
}
}
}
