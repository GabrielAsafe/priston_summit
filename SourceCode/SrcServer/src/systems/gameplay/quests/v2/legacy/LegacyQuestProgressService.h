#pragma once

#include "gameplay/quests/v2/QuestProgressService.h"

namespace gameplay
{
namespace quests
{
namespace legacy
{
class LegacyQuestProgressService final : public IQuestProgressService
{
public:
    bool RecordMonsterKill(PlayerId player,
                           EntityId monster,
                           bool partyCredit) override;
    bool RecordTier5MonsterKill(PlayerId killer,
                                EntityId monster,
                                const std::vector<PlayerId>& damageUsers) override;
};

LegacyQuestProgressService& ServerLegacyQuestProgressService();
}
}
}
