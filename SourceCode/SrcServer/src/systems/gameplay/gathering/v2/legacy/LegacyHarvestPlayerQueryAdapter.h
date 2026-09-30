#pragma once

#include "gameplay/gathering/v2/HarvestPlayerQuery.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

namespace gameplay
{
namespace gathering
{
namespace legacy
{
class LegacyHarvestPlayerQueryAdapter final : public IHarvestPlayerQuery
{
public:
    explicit LegacyHarvestPlayerQueryAdapter(const player::legacy::LegacyOnlinePlayerRegistry& players);

    bool TryGet(PlayerId player, HarvestPlayerSnapshot& out) const override;

private:
    const player::legacy::LegacyOnlinePlayerRegistry& players_;
};

const LegacyHarvestPlayerQueryAdapter& ServerLegacyHarvestPlayerQueryAdapter();
}
}
}
