#pragma once

#include "gameplay/progression/v2/PlayerExperienceService.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

namespace gameplay
{
namespace progression
{
namespace legacy
{
class LegacyPlayerExperienceAdapter final : public IPlayerExperienceMutationPort
{
public:
    explicit LegacyPlayerExperienceAdapter(const player::legacy::LegacyOnlinePlayerRegistry& players);

    PlayerExperienceGrantResult Grant(const PlayerExperienceGrantRequest& request) override;

private:
    const player::legacy::LegacyOnlinePlayerRegistry& players_;
};

LegacyPlayerExperienceAdapter& ServerLegacyPlayerExperienceAdapter();
}
}
}
