#include "systems/gameplay/quests/v2/legacy/LegacyQuestProgressService.h"

#include <cstdint>

#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"
#include "systems/gameplay/quests/server/cQuestHandler.h"

namespace gameplay
{
namespace quests
{
namespace legacy
{
bool LegacyQuestProgressService::RecordMonsterKill(PlayerId player,
                                                    EntityId monster,
                                                    bool partyCredit)
{
    if (!player.Valid() || !monster.Valid())
        return false;

    rsPLAYINFO* playerValue = player::legacy::ServerLegacyOnlinePlayerRegistry().Resolve(player);
    smCHAR* monsterValue = srFindCharFromSerial(static_cast<DWORD>(monster.value));
    if (!playerValue || !monsterValue)
        return false;

    return cQuestHandler.KillMonster(playerValue, monsterValue, partyCredit ? 1 : 0);
}

bool LegacyQuestProgressService::RecordTier5MonsterKill(
    PlayerId killer,
    EntityId monster,
    const std::vector<PlayerId>& damageUsers)
{
    if (!killer.Valid() || !monster.Valid())
        return false;

    rsPLAYINFO* killerValue = player::legacy::ServerLegacyOnlinePlayerRegistry().Resolve(killer);
    smCHAR* monsterValue = srFindCharFromSerial(static_cast<DWORD>(monster.value));
    if (!killerValue || !monsterValue)
        return false;

    std::vector<rsPLAYINFO*> resolvedDamageUsers;
    resolvedDamageUsers.reserve(damageUsers.size());
    for (std::vector<PlayerId>::const_iterator it = damageUsers.begin();
         it != damageUsers.end(); ++it)
    {
        rsPLAYINFO* resolved = player::legacy::ServerLegacyOnlinePlayerRegistry().Resolve(*it);
        if (resolved)
            resolvedDamageUsers.push_back(resolved);
    }

    rsPLAYINFO** damageUserValues = resolvedDamageUsers.empty() ? 0 : &resolvedDamageUsers[0];
    return cQuestHandler.HandleTier5MonsterKill(
        killerValue,
        monsterValue,
        damageUserValues,
        static_cast<int>(resolvedDamageUsers.size()));
}

LegacyQuestProgressService& ServerLegacyQuestProgressService()
{
    static LegacyQuestProgressService service;
    return service;
}
}
}
}
