#pragma once

#include "gameplay/core/v2/GameplayIds.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

namespace gameplay
{
namespace quests
{
namespace legacy
{
class LegacyQuestQueryIngressAdapter
{
public:
    explicit LegacyQuestQueryIngressAdapter(const player::legacy::LegacyOnlinePlayerRegistry& players)
        : players_(players) {}

    bool SendSpoilerInfo(PlayerId player, int spoilerNumber) const;
    bool SendQuestInfo(PlayerId player, int questId, int questKind, int spoilerNumber) const;

private:
    const player::legacy::LegacyOnlinePlayerRegistry& players_;
};

const LegacyQuestQueryIngressAdapter& ServerLegacyQuestQueryIngressAdapter();
}
}
}
