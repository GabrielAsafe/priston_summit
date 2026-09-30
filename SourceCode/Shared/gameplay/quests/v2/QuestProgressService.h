#pragma once

#include "gameplay/core/v2/GameplayIds.h"

#include <vector>

namespace gameplay
{
namespace quests
{
class IQuestProgressService
{
public:
    virtual ~IQuestProgressService() {}

    // Records authoritative quest progress caused by a monster kill.
    // The implementation owns legacy entity/player resolution and objective rules.
    virtual bool RecordMonsterKill(PlayerId player,
                                   EntityId monster,
                                   bool partyCredit) = 0;

    // Preserves the legacy Tier-5 shared-credit rules that inspect the
    // authenticated killer plus the server-selected top damage participants.
    virtual bool RecordTier5MonsterKill(PlayerId killer,
                                        EntityId monster,
                                        const std::vector<PlayerId>& damageUsers) = 0;
};
}
}
