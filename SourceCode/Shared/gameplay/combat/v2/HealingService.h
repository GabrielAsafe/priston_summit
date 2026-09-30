#pragma once

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace combat
{
enum class HealingCode
{
    Ok = 0,
    InvalidRequest,
    TargetNotFound,
    TargetInactive,
    TargetDead,
    InvalidState,
    LegacyRejected
};

struct HealingResult
{
    HealingCode code;
    int requested;
    int before;
    int after;
    int maximum;
    bool delivered;
    bool applied;

    HealingResult()
        : code(HealingCode::InvalidRequest), requested(0), before(0), after(0), maximum(0),
          delivered(false), applied(false) {}

    bool Accepted() const { return code == HealingCode::Ok && delivered; }
};

class IHealingService
{
public:
    virtual ~IHealingService() {}

    virtual HealingResult Heal(EntityId source, EntityId target, int amount) = 0;
    virtual HealingResult Heal(EntityId source, PlayerId target, int amount) = 0;
};
}
}
