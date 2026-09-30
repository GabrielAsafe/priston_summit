#pragma once

#include "gameplay/covenants/v2/CovenantUIIngress.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

namespace gameplay
{
namespace covenants
{
namespace legacy
{
class LegacyCovenantUIIngressAdapter final : public ICovenantUIIngress
{
public:
    explicit LegacyCovenantUIIngressAdapter(const player::legacy::LegacyOnlinePlayerRegistry& players);

    void HandleAction(PlayerId owner,
                      const protocol::CovenantUIActionPacket& packet) override;

private:
    const player::legacy::LegacyOnlinePlayerRegistry& players_;
};

LegacyCovenantUIIngressAdapter& ServerLegacyCovenantUIIngressAdapter();
}
}
}
