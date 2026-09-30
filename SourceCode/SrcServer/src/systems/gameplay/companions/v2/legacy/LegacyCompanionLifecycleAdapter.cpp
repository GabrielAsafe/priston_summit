#include "systems/gameplay/companions/v2/legacy/LegacyCompanionLifecycleAdapter.h"

#include "systems/gameplay/companions/v2/legacy/CompanionLegacyLifecycleRuntime.h"

namespace gameplay
{
namespace companions
{
namespace legacy
{
LegacyCompanionLifecycleAdapter::LegacyCompanionLifecycleAdapter(
    const player::legacy::LegacyOnlinePlayerRegistry& players)
    : players_(players)
{
}

void LegacyCompanionLifecycleAdapter::OwnerDisconnected(PlayerHandle owner)
{
    rsPLAYINFO* legacyOwner = players_.Resolve(owner);
    if(legacyOwner)
        companions::OnOwnerDisconnect(legacyOwner);
}

LegacyCompanionLifecycleAdapter& ServerLegacyCompanionLifecycleAdapter()
{
    static LegacyCompanionLifecycleAdapter adapter(player::legacy::ServerLegacyOnlinePlayerRegistry());
    return adapter;
}
}
}
}
