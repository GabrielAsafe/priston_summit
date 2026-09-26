#include "systems/gameplay/events/v2/GameplayEventRuntime.h"

#include <cstdint>
#include <cstdlib>
#include <sstream>

#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "systems/gameplay/events/v2/MonsterKilledEvent.h"
#include "systems/gameplay/hunting/v2/HuntingService.h"
#include "systems/gameplay/quests/server/cQuestHandler.h"
#include "systems/gameplay/progression/v2/ProgressionRuntime.h"
#include "gameplay/covenants/v2/CovenantCatalog.h"

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

    static const GameplayEventHub::SubscriptionId covenantSubscription =
        ServerGameplayEventHub().Subscribe<MonsterKilledEvent>(
            [](const MonsterKilledEvent& event)
            {
                if (event.playerName.empty())
                    return;

                // Fortune/Ascendants progress on ordinary monster kills. Hunters
                // use boss/special monsters as the V1 Hunt Target bootstrap until
                // the data-driven Hunt catalog provides explicit target tags.
                (void)progression::RecordServerCovenantActivity(
                    event.playerName, "monster.kill", 1);
                if (event.monster &&
                    event.monster->smCharInfo.wPlayClass[0] == MONSTER_CLASS_BOSS)
                {
                    (void)progression::RecordServerCovenantActivity(
                        event.playerName, "hunt.kill", 1);

                    // Hunter trophies live in the Covenant inventory until an
                    // explicit legacy-item adapter is configured. This makes the
                    // economy/progression functional without inventing item IDs.
                    covenants::CovenantModifierAggregate trophyChance;
                    if (progression::GetServerCovenantModifierAggregate(
                            event.playerName, "hunt.trophy_chance", "hunt_target",
                            trophyChance, true, event.monster->dwObjectSerial) &&
                        trophyChance.matched)
                    {
                        double chance = trophyChance.flat + trophyChance.additivePercent;
                        if (trophyChance.multiplicativeFactor != 1.0)
                            chance *= trophyChance.multiplicativeFactor;
                        if (chance > 1.0) chance = 1.0;
                        if (chance > 0.0 && (std::rand() % 10000) < static_cast<int>(chance * 10000.0))
                        {
                            std::ostringstream trophyKey;
                            trophyKey << "trophy:" << event.monsterCode;
                            (void)progression::AddServerCovenantInventoryItem(
                                event.playerName, covenants::catalog::kHunters, trophyKey.str(), 1);
                        }
                    }
                }
            });

    (void)legacyQuestSubscription;
    (void)huntingSubscription;
    (void)covenantSubscription;
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
