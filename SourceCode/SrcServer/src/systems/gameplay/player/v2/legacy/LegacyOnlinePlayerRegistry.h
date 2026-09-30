#pragma once

#include <string>
#include <vector>

#include "gameplay/core/v2/GameplayIds.h"

class rsPLAYINFO;

namespace gameplay
{
namespace player
{
namespace legacy
{
class LegacyOnlinePlayerRegistry
{
public:
    PlayerId IdOf(const rsPLAYINFO* player) const;
    PlayerHandle HandleOf(const rsPLAYINFO* player) const;

    rsPLAYINFO* Resolve(PlayerId player) const;
    rsPLAYINFO* Resolve(PlayerHandle player) const;
    rsPLAYINFO* ResolveByPlayerKey(const std::string& playerKey) const;
    std::vector<PlayerHandle> OnlineHandles() const;

    bool IsOnline(PlayerId player) const;
    bool IsCurrent(PlayerHandle player) const;
};

const LegacyOnlinePlayerRegistry& ServerLegacyOnlinePlayerRegistry();
}
}
}
