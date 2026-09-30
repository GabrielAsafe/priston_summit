#pragma once

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace combat
{
enum class CombatBuffCode
{
    Ok = 0,
    InvalidRequest,
    TargetNotFound,
    TargetInactive
};

struct CombatBuffResult
{
    CombatBuffCode code;
    bool applied;

    CombatBuffResult() : code(CombatBuffCode::InvalidRequest), applied(false) {}
    bool Ok() const { return code == CombatBuffCode::Ok && applied; }
};

struct CombatBuffDelta
{
    int attackMin;
    int attackMax;
    int attackRating;
    int defence;
    int attackSpeed;
    int shootingRange;
    int absorption;
    int chanceBlock;
    int maxLife;
    float lifeRegen;
    float manaRegen;
    int resistance[8];

    CombatBuffDelta()
        : attackMin(0), attackMax(0), attackRating(0), defence(0), attackSpeed(0),
          shootingRange(0), absorption(0), chanceBlock(0), maxLife(0),
          lifeRegen(0.0f), manaRegen(0.0f)
    {
        for(int i = 0; i < 8; ++i)
            resistance[i] = 0;
    }
};

class ICombatBuffService
{
public:
    virtual ~ICombatBuffService() {}
    virtual CombatBuffResult Apply(EntityId target, const CombatBuffDelta& delta) = 0;
    virtual CombatBuffResult Remove(EntityId target, const CombatBuffDelta& delta) = 0;
};
}
}
