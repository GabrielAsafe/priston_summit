#pragma once

#include "gameplay/quests/v2/QuestObjectiveService.h"

namespace gameplay
{
namespace quests
{
namespace legacy
{
class LegacyQuestObjectiveService final : public IQuestObjectiveService
{
public:
    bool IsInProgress(PlayerId player, QuestId quest) override;
    bool HasPendingObjectives(PlayerId player, QuestId quest) override;
    bool CompleteObjectives(PlayerId player, QuestId quest) override;
    bool CancelProgress(PlayerId player, QuestId quest) override;
};

LegacyQuestObjectiveService& ServerLegacyQuestObjectiveService();
}
}
}
