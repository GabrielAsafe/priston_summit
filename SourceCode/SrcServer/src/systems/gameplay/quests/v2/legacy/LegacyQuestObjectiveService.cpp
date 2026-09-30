#include "systems/gameplay/quests/v2/legacy/LegacyQuestObjectiveService.h"

#include <string>

#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"
#include "systems/gameplay/quests/v2/legacy/LegacyQuestRepository.h"

namespace gameplay
{
namespace quests
{
namespace legacy
{
namespace
{
bool ResolvePlayerKey(PlayerId player, std::string& playerKey)
{
    playerKey.clear();
    if(!player.Valid())
        return false;

    rsPLAYINFO* resolved = player::legacy::ServerLegacyOnlinePlayerRegistry().Resolve(player);
    if(!resolved || !resolved->getName() || !resolved->getName()[0])
        return false;

    playerKey = resolved->getName();
    return true;
}
}

bool LegacyQuestObjectiveService::IsInProgress(PlayerId player, QuestId quest)
{
    std::string playerKey;
    bool inProgress = false;
    return quest.Valid() && ResolvePlayerKey(player, playerKey) &&
        ServerLegacyQuestRepository().TryIsInProgress(playerKey, quest, inProgress) && inProgress;
}

bool LegacyQuestObjectiveService::HasPendingObjectives(PlayerId player, QuestId quest)
{
    std::string playerKey;
    bool pending = false;
    return quest.Valid() && ResolvePlayerKey(player, playerKey) &&
        ServerLegacyQuestRepository().TryHasPendingObjectives(playerKey, quest, pending) && pending;
}

bool LegacyQuestObjectiveService::CompleteObjectives(PlayerId player, QuestId quest)
{
    std::string playerKey;
    return quest.Valid() && ResolvePlayerKey(player, playerKey) &&
        ServerLegacyQuestRepository().CompleteObjectives(playerKey, quest);
}

bool LegacyQuestObjectiveService::CancelProgress(PlayerId player, QuestId quest)
{
    std::string playerKey;
    return quest.Valid() && ResolvePlayerKey(player, playerKey) &&
        ServerLegacyQuestRepository().CancelProgress(playerKey, quest);
}

LegacyQuestObjectiveService& ServerLegacyQuestObjectiveService()
{
    static LegacyQuestObjectiveService service;
    return service;
}
}
}
}
