#include "systems/gameplay/quests/v2/legacy/LegacyQuestStateQuery.h"

#include "systems/gameplay/quests/v2/legacy/LegacyQuestRepository.h"

namespace gameplay
{
namespace quests
{
namespace legacy
{
bool LegacyQuestStateQuery::TryGetState(const std::string& playerKey,
                                        QuestId quest,
                                        QuestStateSnapshot& out) const
{
    if(playerKey.empty() || !quest.Valid())
        return false;

    bool completed = false;
    if(!repository_.TryIsCompleted(playerKey, quest, completed))
        return false;

    bool inProgress = false;
    // Preserve the legacy reader's short-circuit: a completed quest is readable even if
    // the Progress table cannot be queried, and Completion wins if both tables contain rows.
    if(!completed && !repository_.TryIsInProgress(playerKey, quest, inProgress))
        return false;

    QuestStateSnapshot snapshot;
    snapshot.quest = quest;
    snapshot.completed = completed;
    snapshot.inProgress = !completed && inProgress;
    out = snapshot;
    return true;
}

const LegacyQuestStateQuery& ServerLegacyQuestStateQuery()
{
    static LegacyQuestStateQuery query(ServerLegacyQuestRepository());
    return query;
}
}
}
}
