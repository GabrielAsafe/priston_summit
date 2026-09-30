#pragma once

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace quests
{
struct QuestDefinition
{
    QuestId id;
    int kind;
    int type;
    int partyFlag;

    QuestDefinition() : id(), kind(0), type(0), partyFlag(0) {}
};

class IQuestCatalog
{
public:
    virtual ~IQuestCatalog() {}
    virtual bool TryGetDefinition(QuestId quest, QuestDefinition& out) const = 0;
};
}
}
