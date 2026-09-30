#include "systems/gameplay/quests/v2/legacy/LegacyQuestQueryIngressAdapter.h"

#include "systems/gameplay/quests/server/cQuestHandler.h"

namespace gameplay
{
namespace quests
{
namespace legacy
{
bool LegacyQuestQueryIngressAdapter::SendSpoilerInfo(PlayerId player, int spoilerNumber) const
{
    rsPLAYINFO* resolved = players_.Resolve(player);
    if(!resolved)
        return false;
    cQuestHandler.GetSpoilerInfo(resolved, spoilerNumber);
    return true;
}

bool LegacyQuestQueryIngressAdapter::SendQuestInfo(PlayerId player,
                                                   int questId,
                                                   int questKind,
                                                   int spoilerNumber) const
{
    rsPLAYINFO* resolved = players_.Resolve(player);
    if(!resolved)
        return false;
    cQuestHandler.GetQuestInfo(resolved, questId, questKind, spoilerNumber);
    return true;
}

const LegacyQuestQueryIngressAdapter& ServerLegacyQuestQueryIngressAdapter()
{
    static LegacyQuestQueryIngressAdapter adapter(player::legacy::ServerLegacyOnlinePlayerRegistry());
    return adapter;
}
}
}
}
