#include "systems/gameplay/quests/v2/legacy/LegacyQuestCommands.h"

#include "systems/gameplay/quests/server/cQuestHandler.h"

namespace gameplay
{
namespace quests
{
namespace legacy
{
bool LegacyQuestCommands::Start(PlayerId player, const QuestStartCommand& command)
{
    rsPLAYINFO* resolved = players_.Resolve(player);
    if(!resolved || !command.quest.Valid())
        return false;

    cQuestHandler.StartQuest(resolved,
                             static_cast<int>(command.quest.value),
                             command.kind,
                             command.type,
                             command.partyFlag);
    return true;
}

bool LegacyQuestCommands::Cancel(PlayerId player, const QuestCommand& command)
{
    rsPLAYINFO* resolved = players_.Resolve(player);
    if(!resolved || !command.quest.Valid())
        return false;

    cQuestHandler.CancelQuest(resolved,
                              static_cast<int>(command.quest.value),
                              command.kind,
                              command.type);
    return true;
}

bool LegacyQuestCommands::ClaimReward(PlayerId player, const QuestCommand& command)
{
    rsPLAYINFO* resolved = players_.Resolve(player);
    if(!resolved || !command.quest.Valid())
        return false;

    cQuestHandler.GetReward(resolved,
                            static_cast<int>(command.quest.value),
                            command.kind,
                            command.type);
    return true;
}

LegacyQuestCommands& ServerLegacyQuestCommands()
{
    static LegacyQuestCommands commands(player::legacy::ServerLegacyOnlinePlayerRegistry());
    return commands;
}
}
}
}
