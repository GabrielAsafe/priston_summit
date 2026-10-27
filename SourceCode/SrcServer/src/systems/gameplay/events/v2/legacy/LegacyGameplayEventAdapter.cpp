#include "systems/gameplay/events/v2/legacy/LegacyGameplayEventAdapter.h"

#include <cstdint>

extern bool SummitVisualNodeIsManagedSerial(std::uint32_t serial);

#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"
#include "systems/gameplay/events/v2/GameplayEventRuntime.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"
#include "systems/gameplay/quests/v2/legacy/LegacyQuestProgressService.h"
#include "systems/gameplay/quests/v2/legacy/CanonicalQuestExecution351C.h"
#include <ctime>

namespace gameplay
{
namespace events
{
namespace legacy
{
namespace
{
void EnsureLegacyQuestConsumerRegistered()
{
    static const GameplayEventHub::SubscriptionId legacyQuestSubscription =
        ServerGameplayEventHub().Subscribe<MonsterKilledEvent>(
            [](const MonsterKilledEvent& event)
            {
                quests::legacy::ServerLegacyQuestProgressService().RecordMonsterKill(
                    event.player, event.monster, event.partyCredit);
                (void)quests::canonical::RecordKills351C(event.playerName,
                    event.monsterName,event.monsterBoss,event.partyCredit,std::time(0));
            });
    (void)legacyQuestSubscription;
}
}

void LegacyGameplayEventAdapter::PublishMonsterKilled(rsPLAYINFO* playerValue,
                                                       smCHAR* monsterValue,
                                                       bool partyCredit)
{
    if (!playerValue || !monsterValue)
        return;

    const PlayerId playerId = player::legacy::ServerLegacyOnlinePlayerRegistry().IdOf(playerValue);
    const EntityId monsterId(static_cast<std::uint32_t>(monsterValue->dwObjectSerial));
    if (!playerId.Valid() || !monsterId.Valid())
        return;

    // The node visual is a harvesting tool, not an ordinary monster kill.
    if (SummitVisualNodeIsManagedSerial(static_cast<std::uint32_t>(monsterValue->dwObjectSerial)))
        return;

    EnsureLegacyQuestConsumerRegistered();

    const char* playerName = playerValue->getName();
    const char* monsterName = monsterValue->smCharInfo.szName;
    const bool isBoss = monsterValue->smCharInfo.wPlayClass[0] == MONSTER_CLASS_BOSS;
    const MonsterKilledEvent event(
        playerId,
        monsterId,
        partyCredit,
        isBoss,
        playerName ? playerName : "",
        monsterName ? monsterName : "",
        static_cast<std::uint32_t>(monsterValue->smMonsterInfo.dwAutoCharCode),
        playerValue->Position.Area);

    gameplay::events::PublishMonsterKilled(event);
}

LegacyGameplayEventAdapter& ServerLegacyGameplayEventAdapter()
{
    static LegacyGameplayEventAdapter adapter;
    return adapter;
}
}
}
}
