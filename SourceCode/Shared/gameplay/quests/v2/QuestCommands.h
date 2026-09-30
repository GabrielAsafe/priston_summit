#pragma once

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace quests
{
struct QuestStartCommand
{
    QuestId quest;
    int kind;
    int type;
    int partyFlag;

    QuestStartCommand() : quest(), kind(0), type(0), partyFlag(0) {}
};

struct QuestCommand
{
    QuestId quest;
    int kind;
    int type;

    QuestCommand() : quest(), kind(0), type(0) {}
};

class IQuestCommands
{
public:
    virtual ~IQuestCommands() {}

    // true means the command reached the authoritative legacy quest runtime.
    // Callers that need the resulting state must query IQuestStateQuery.
    virtual bool Start(PlayerId player, const QuestStartCommand& command) = 0;
    virtual bool Cancel(PlayerId player, const QuestCommand& command) = 0;
    virtual bool ClaimReward(PlayerId player, const QuestCommand& command) = 0;
};
}
}
