#pragma once

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace quests
{
class IQuestObjectiveService
{
public:
    virtual ~IQuestObjectiveService() {}

    virtual bool IsInProgress(PlayerId player, QuestId quest) = 0;
    virtual bool HasPendingObjectives(PlayerId player, QuestId quest) = 0;
    virtual bool CompleteObjectives(PlayerId player, QuestId quest) = 0;
    virtual bool CancelProgress(PlayerId player, QuestId quest) = 0;
};
}
}
