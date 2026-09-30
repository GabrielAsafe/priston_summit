#pragma once

#include "gameplay/events/v2/WorldEventAdmission.h"
#include "gameplay/player/v2/IPlayerQuery.h"
#include "network/v2/ConnectionQuery.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

namespace gameplay
{
namespace events
{
namespace legacy
{
class LegacyWorldEventAdmissionAdapter final : public IWorldEventAdmissionService
{
public:
    LegacyWorldEventAdmissionAdapter(
        const player::legacy::LegacyOnlinePlayerRegistry& onlinePlayers,
        const network::IConnectionQuery& connections,
        const player::IPlayerQuery& players);

    WorldEventAdmissionStatus Check(const WorldEventAdmissionRequest& request) const override;

private:
    const player::legacy::LegacyOnlinePlayerRegistry& onlinePlayers_;
    const network::IConnectionQuery& connections_;
    const player::IPlayerQuery& players_;
};

const LegacyWorldEventAdmissionAdapter& ServerLegacyWorldEventAdmissionAdapter();
}
}
}
