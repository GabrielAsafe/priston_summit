#include "systems/gameplay/quests/v2/legacy/LegacyQuestAdminService.h"

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
bool LegacyQuestAdminService::MarkCompletedForTesting(PlayerId player, QuestId quest)
{
    if (!player.Valid() || !quest.Valid())
        return false;

    rsPLAYINFO* resolved = player::legacy::ServerLegacyOnlinePlayerRegistry().Resolve(player);
    return resolved && resolved->getName() && resolved->getName()[0] &&
        ServerLegacyQuestRepository().MarkSingleCompletedForTesting(resolved->getName(), quest);
}

LegacyQuestAdminService& ServerLegacyQuestAdminService()
{
    static LegacyQuestAdminService service;
    return service;
}
}
}
}
