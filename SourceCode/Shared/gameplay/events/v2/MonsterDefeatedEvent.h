#pragma once

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace events
{
struct MonsterDefeatedEvent
{
    EntityId monster;

    explicit MonsterDefeatedEvent(EntityId monsterValue = EntityId())
        : monster(monsterValue)
    {
    }
};
}
}
