#pragma once

#include "gameplay/companions/v2/CompanionLifecycle.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

namespace gameplay
{
namespace companions
{
namespace legacy
{
class LegacyCompanionLifecycleAdapter final : public ICompanionLifecycle
{
public:
    explicit LegacyCompanionLifecycleAdapter(const player::legacy::LegacyOnlinePlayerRegistry& players);
    void OwnerDisconnected(PlayerHandle owner) override;

private:
    const player::legacy::LegacyOnlinePlayerRegistry& players_;
};

LegacyCompanionLifecycleAdapter& ServerLegacyCompanionLifecycleAdapter();
}
}
}
