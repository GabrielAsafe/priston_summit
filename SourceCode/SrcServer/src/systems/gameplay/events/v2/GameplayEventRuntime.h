#pragma once

#include "gameplay/events/v2/GameplayEventHub.h"
#include "gameplay/events/v2/MonsterDefeatedEvent.h"
#include "systems/gameplay/events/v2/MonsterKilledEvent.h"
#include "gameplay/events/v2/WorldEventLifecycleEvents.h"

namespace gameplay
{
namespace events
{
GameplayEventHub& ServerGameplayEventHub();
void PublishMonsterDefeated(const MonsterDefeatedEvent& event);
void PublishMonsterKilled(const MonsterKilledEvent& event);
void PublishWorldEventStarted(const WorldEventStarted& event);
void PublishEventScoreChanged(const EventScoreChanged& event);
void PublishWorldEventEnded(const WorldEventEnded& event);
}
}
