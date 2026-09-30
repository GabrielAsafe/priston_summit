#include "systems/gameplay/companions/v2/legacy/LegacyCompanionCommandsAdapter.h"

#include "systems/gameplay/companions/v2/legacy/CompanionLegacyCommandsRuntime.h"

namespace gameplay
{
namespace companions
{
namespace legacy
{
namespace
{
CompanionMode ToLegacyMode(CompanionBehaviorMode mode)
{
    switch(mode)
    {
        case CompanionBehaviorMode::Follow: return COMPANION_MODE_FOLLOW;
        case CompanionBehaviorMode::Passive: return COMPANION_MODE_PASSIVE;
        default: return COMPANION_MODE_AGGRESSIVE;
    }
}
}

LegacyCompanionCommandsAdapter::LegacyCompanionCommandsAdapter(
    const player::legacy::LegacyOnlinePlayerRegistry& players)
    : players_(players)
{
}

int LegacyCompanionCommandsAdapter::Select(PlayerId owner, const std::string& characterName)
{
    rsPLAYINFO* legacyOwner = players_.Resolve(owner);
    return legacyOwner ? companions::Select(legacyOwner, characterName.c_str()) : 0;
}

int LegacyCompanionCommandsAdapter::Summon(PlayerId owner, const std::string& characterName)
{
    rsPLAYINFO* legacyOwner = players_.Resolve(owner);
    return legacyOwner ? companions::Summon(legacyOwner, characterName.c_str()) : 0;
}

int LegacyCompanionCommandsAdapter::SummonSelected(PlayerId owner)
{
    rsPLAYINFO* legacyOwner = players_.Resolve(owner);
    return legacyOwner ? companions::SummonSelected(legacyOwner) : 0;
}

int LegacyCompanionCommandsAdapter::Dismiss(PlayerId owner, const std::string& characterName)
{
    rsPLAYINFO* legacyOwner = players_.Resolve(owner);
    return legacyOwner ? companions::Dismiss(legacyOwner, characterName.c_str()) : 0;
}

int LegacyCompanionCommandsAdapter::DismissAll(PlayerId owner)
{
    rsPLAYINFO* legacyOwner = players_.Resolve(owner);
    return legacyOwner ? companions::DismissAll(legacyOwner) : 0;
}

int LegacyCompanionCommandsAdapter::SetMode(PlayerId owner, CompanionBehaviorMode mode)
{
    rsPLAYINFO* legacyOwner = players_.Resolve(owner);
    return legacyOwner ? companions::SetMode(legacyOwner, ToLegacyMode(mode)) : 0;
}

int LegacyCompanionCommandsAdapter::UseCrystal(PlayerId owner, std::uint32_t itemCode)
{
    rsPLAYINFO* legacyOwner = players_.Resolve(owner);
    return legacyOwner ? companions::HandleCrystalUse(legacyOwner, itemCode) : 0;
}

LegacyCompanionCommandsAdapter& ServerLegacyCompanionCommandsAdapter()
{
    static LegacyCompanionCommandsAdapter adapter(player::legacy::ServerLegacyOnlinePlayerRegistry());
    return adapter;
}
}
}
}
