#pragma once

#include <string>

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace quests
{
struct QuestStateSnapshot
{
    QuestId quest;
    bool completed;
    bool inProgress;

    QuestStateSnapshot() : quest(), completed(false), inProgress(false) {}

    bool Active() const { return inProgress && !completed; }
};

class IQuestStateQuery
{
public:
    virtual ~IQuestStateQuery() {}

    // playerKey is the persistence identity used by the legacy QuestDB (character name).
    // false means the authoritative state could not be read; callers must not interpret
    // that as NotActive.
    virtual bool TryGetState(const std::string& playerKey,
                             QuestId quest,
                             QuestStateSnapshot& out) const = 0;
};
}
}
