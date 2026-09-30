#pragma once

#include "gameplay/companions/v2/CompanionCommands.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

namespace gameplay
{
namespace companions
{
namespace legacy
{
class LegacyCompanionCommandsAdapter final : public ICompanionCommands
{
public:
    explicit LegacyCompanionCommandsAdapter(const player::legacy::LegacyOnlinePlayerRegistry& players);

    int Select(PlayerId owner, const std::string& characterName) override;
    int Summon(PlayerId owner, const std::string& characterName) override;
    int SummonSelected(PlayerId owner) override;
    int Dismiss(PlayerId owner, const std::string& characterName) override;
    int DismissAll(PlayerId owner) override;
    int SetMode(PlayerId owner, CompanionBehaviorMode mode) override;
    int UseCrystal(PlayerId owner, std::uint32_t itemCode) override;

private:
    const player::legacy::LegacyOnlinePlayerRegistry& players_;
};

LegacyCompanionCommandsAdapter& ServerLegacyCompanionCommandsAdapter();
}
}
}
