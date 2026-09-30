#pragma once

#include "gameplay/companions/v2/CompanionUIIngress.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

namespace gameplay
{
namespace companions
{
namespace legacy
{
class LegacyCompanionUIIngressAdapter final : public ICompanionUIIngress
{
public:
    explicit LegacyCompanionUIIngressAdapter(const player::legacy::LegacyOnlinePlayerRegistry& players);
    void HandleAction(PlayerId owner, const protocol::CompanionUIActionPacket& packet) override;

private:
    const player::legacy::LegacyOnlinePlayerRegistry& players_;
};

LegacyCompanionUIIngressAdapter& ServerLegacyCompanionUIIngressAdapter();
}
}
}
