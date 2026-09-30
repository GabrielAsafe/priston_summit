#pragma once

#include "network/v2/ConnectionQuery.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

namespace gameplay
{
namespace network
{
namespace legacy
{
class LegacyConnectionQuery final : public IConnectionQuery
{
public:
    explicit LegacyConnectionQuery(const player::legacy::LegacyOnlinePlayerRegistry& players);
    bool TryGet(PlayerId player, ConnectionSnapshot& out) const override;
    bool IsConnected(PlayerId player) const override;
    bool IsCurrent(PlayerHandle player) const override;

private:
    const player::legacy::LegacyOnlinePlayerRegistry& players_;
};

const LegacyConnectionQuery& ServerLegacyConnectionQuery();
}
}
}
