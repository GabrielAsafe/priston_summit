#pragma once

#include <string>

#include "gameplay/quests/v2/QuestCommands.h"
#include "gameplay/quests/v2/QuestStateQuery.h"

namespace gameplay
{
namespace quests
{
class QuestService
{
public:
    QuestService(const IQuestStateQuery& states, IQuestCommands& commands)
        : states_(states), commands_(commands) {}

    // Idempotent orchestration used by Story/Progression: already-active is success,
    // completed/unreadable/rejected is failure, and a new start must be observable
    // as InProgress before success is reported.
    bool EnsureStarted(PlayerId player,
                       const std::string& playerKey,
                       const QuestStartCommand& command)
    {
        if(!player.Valid() || playerKey.empty() || !command.quest.Valid())
            return false;

        QuestStateSnapshot state;
        if(!states_.TryGetState(playerKey, command.quest, state))
            return false;
        if(state.completed)
            return false;
        if(state.inProgress)
            return true;

        if(!commands_.Start(player, command))
            return false;

        if(!states_.TryGetState(playerKey, command.quest, state))
            return false;
        return !state.completed && state.inProgress;
    }

private:
    const IQuestStateQuery& states_;
    IQuestCommands& commands_;
};
}
}
