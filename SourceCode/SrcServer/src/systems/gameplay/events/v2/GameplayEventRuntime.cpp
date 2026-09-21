#include "systems/gameplay/events/v2/GameplayEventRuntime.h"

#include <cstdint>

#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "systems/gameplay/events/v2/MonsterKilledEvent.h"
#include "systems/gameplay/hunting/v2/HuntingService.h"
#include "systems/gameplay/quests/server/cQuestHandler.h"

namespace gameplay
{
namespace events
{
namespace
{
void EnsureCoreConsumersRegistered()
{
    static const GameplayEventHub::SubscriptionId legacyQuestSubscription =
        ServerGameplayEventHub().Subscribe<MonsterKilledEvent>(
            [](const MonsterKilledEvent& event)
            {
                if (event.player && event.monster)
                    cQuestHandler.KillMonster(event.player, event.monster, event.partyCredit ? 1 : 0);
            });

    static const GameplayEventHub::SubscriptionId huntingSubscription =
        ServerGameplayEventHub().Subscribe<MonsterKilledEvent>(
            [](const MonsterKilledEvent& event)
            {
                hunting::ServerHuntingService().OnMonsterKilled(event);
            });

    (void)legacyQuestSubscription;
    (void)huntingSubscription;
}
}

GameplayEventHub& ServerGameplayEventHub()
{
    static GameplayEventHub hub;
    return hub;
}

void PublishMonsterKilled(rsPLAYINFO* player, smCHAR* monster, bool partyCredit)
{
    if (!player || !monster)
        return;

    EnsureCoreConsumersRegistered();

    const char* playerName = player->getName();
    const char* monsterName = monster->smCharInfo.szName;
    const MonsterKilledEvent event(
        player,
        monster,
        partyCredit,
        playerName ? playerName : "",
        monsterName ? monsterName : "",
        static_cast<std::uint32_t>(monster->smMonsterInfo.dwAutoCharCode),
        player->Position.Area);

    ServerGameplayEventHub().Publish(event);
}
}
}
