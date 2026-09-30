#include "systems/gameplay/events/v2/GameplayEventRuntime.h"

#include <cstdlib>
#include <sstream>

#include "systems/gameplay/hunting/v2/HuntingService.h"
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

                (void)progression::RecordServerCovenantActivity(
                    event.playerName, "monster.kill", 1);
                if (event.monsterBoss)
                {
                    (void)progression::RecordServerCovenantActivity(
                        event.playerName, "hunt.kill", 1);

                    covenants::CovenantModifierAggregate trophyChance;
                    if (progression::GetServerCovenantModifierAggregate(
                            event.playerName, "hunt.trophy_chance", "hunt_target",
                            trophyChance, true, event.monster.value) &&
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

    (void)huntingSubscription;
    (void)covenantSubscription;
}
}

GameplayEventHub& ServerGameplayEventHub()
{
    static GameplayEventHub hub;
    return hub;
}

void PublishMonsterDefeated(const MonsterDefeatedEvent& event)
{
    if (!event.monster.Valid())
        return;
    ServerGameplayEventHub().Publish(event);
}

void PublishMonsterKilled(const MonsterKilledEvent& event)
{
    if (!event.player.Valid() || !event.monster.Valid())
        return;

    EnsureCoreConsumersRegistered();
    ServerGameplayEventHub().Publish(event);
}
void PublishWorldEventStarted(const WorldEventStarted& event)
{
    if(event.eventKey.empty())
        return;
    ServerGameplayEventHub().Publish(event);
}

void PublishEventScoreChanged(const EventScoreChanged& event)
{
    if(event.eventKey.empty() || !event.player.Valid())
        return;
    ServerGameplayEventHub().Publish(event);
}

void PublishWorldEventEnded(const WorldEventEnded& event)
{
    if(event.eventKey.empty())
        return;
    ServerGameplayEventHub().Publish(event);
}

}
}
