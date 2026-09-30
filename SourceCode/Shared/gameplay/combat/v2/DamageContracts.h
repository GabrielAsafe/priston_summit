#pragma once

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace combat
{
enum class DamageRejectReason
{
    None = 0,
    InvalidRequest,
    SourceNotFound,
    OwnerNotFound,
    TargetNotFound,
    TargetInactive,
    TargetDead,
    TargetNotEnemy,
    TargetIsPlayerMonster,
    TargetIsOwnedMonster
};

struct DamageRequest
{
    EntityId source;
    PlayerId owner;
    EntityId target;
    int rawDamage;

    DamageRequest(EntityId sourceValue = EntityId(),
                  PlayerId ownerValue = PlayerId(),
                  EntityId targetValue = EntityId(),
                  int rawDamageValue = 0)
        : source(sourceValue), owner(ownerValue), target(targetValue), rawDamage(rawDamageValue) {}
};

struct DamageResult
{
    DamageRejectReason rejectReason;
    int rawDamage;
    int finalDamage;
    int remainingLife;
    bool applied;
    bool killed;
    bool virtualLifeTriggered;

    DamageResult()
        : rejectReason(DamageRejectReason::InvalidRequest), rawDamage(0), finalDamage(0),
          remainingLife(0), applied(false), killed(false), virtualLifeTriggered(false) {}

    bool Accepted() const { return rejectReason == DamageRejectReason::None && applied; }
};

class IDamageService
{
public:
    virtual ~IDamageService() {}
    virtual DamageResult Apply(const DamageRequest& request) = 0;
};
}
}
