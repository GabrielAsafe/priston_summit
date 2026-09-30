#include "systems/gameplay/quests/v2/legacy/LegacyQuestLifecycleService.h"

#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"
#include "systems/gameplay/quests/v2/legacy/LegacyQuestCommands.h"
#include "systems/gameplay/quests/v2/legacy/LegacyQuestRepository.h"

namespace gameplay
{
namespace quests
{
namespace legacy
{
void LegacyQuestLifecycleService::VerifyPlayerLevel(PlayerId player)
{
    if(!player.Valid())
        return;

    rsPLAYINFO* resolved = player::legacy::ServerLegacyOnlinePlayerRegistry().Resolve(player);
    if(!resolved || !resolved->getName() || !resolved->getName()[0])
        return;

    std::vector<QuestProgressRecord> invalid;
    if(!ServerLegacyQuestRepository().ListInvalidForLevel(
           resolved->getName(), resolved->smCharInfo.Level, invalid))
        return;

    for(std::vector<QuestProgressRecord>::const_iterator it = invalid.begin();
        it != invalid.end(); ++it)
    {
        QuestCommand command;
        command.quest = it->quest;
        command.kind = it->kind;
        command.type = it->type;
        ServerLegacyQuestCommands().Cancel(player, command);
    }
}

LegacyQuestLifecycleService& ServerLegacyQuestLifecycleService()
{
    static LegacyQuestLifecycleService service;
    return service;
}
}
}
}
