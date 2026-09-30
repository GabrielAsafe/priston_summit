#pragma once

#include "gameplay/quests/v2/QuestCommands.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

namespace gameplay
{
namespace quests
{
namespace legacy
{
class LegacyQuestCommands final : public IQuestCommands
{
public:
    explicit LegacyQuestCommands(const player::legacy::LegacyOnlinePlayerRegistry& players)
        : players_(players) {}

    bool Start(PlayerId player, const QuestStartCommand& command) override;
    bool Cancel(PlayerId player, const QuestCommand& command) override;
    bool ClaimReward(PlayerId player, const QuestCommand& command) override;

private:
    const player::legacy::LegacyOnlinePlayerRegistry& players_;
};

LegacyQuestCommands& ServerLegacyQuestCommands();
}
}
}
