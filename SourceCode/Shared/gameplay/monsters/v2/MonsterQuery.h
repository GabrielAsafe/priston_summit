#pragma once

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace monsters
{
enum class MonsterFamily
{
    Unknown = 0,
    Normal,
    Undead,
    Mutant,
    Demon,
    Mechanic,
    PlayerFollower
};

struct MonsterSnapshot
{
    EntityId entity;
    bool active;
    bool hostile;
    bool controlled;
    MonsterFamily family;
    int level;
    int life;
    int maxLife;
    int bionicResistance;
    int iceResistance;

    MonsterSnapshot()
        : entity(), active(false), hostile(false), controlled(false),
          family(MonsterFamily::Unknown), level(0), life(0), maxLife(0),
          bionicResistance(0), iceResistance(0) {}
};

class IMonsterQuery
{
public:
    virtual ~IMonsterQuery() {}
    virtual bool TryGetMonster(EntityId entity, MonsterSnapshot& out) const = 0;
};
}
}
