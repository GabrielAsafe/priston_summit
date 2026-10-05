#pragma once

#include <cstdint>

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace combat
{
enum class StatusEffectCode
{
    Ok = 0,
    InvalidRequest,
    TargetNotFound,
    TargetInactive,
    TargetDead
};

enum class StatusEffectKind
{
    Stun = 0
};

enum class SlowTimerMode
{
    CountDownToZero = 0,
    CountUpFromNegative
};

struct StatusEffectResult
{
    StatusEffectCode code;
    bool applied;

    StatusEffectResult() : code(StatusEffectCode::InvalidRequest), applied(false) {}
    bool Ok() const { return code == StatusEffectCode::Ok && applied; }
};

struct SlowEffect
{
    int durationTicks;
    int moveSpeed;
    int distortionTicks;
    bool updateAttackIce;
    bool attackIce;
    bool clearStun;
    SlowTimerMode timerMode;

    SlowEffect(int durationTicksValue = 0, int moveSpeedValue = 0,
               int distortionTicksValue = 0, bool updateAttackIceValue = false,
               bool attackIceValue = false, bool clearStunValue = false,
               SlowTimerMode timerModeValue = SlowTimerMode::CountDownToZero)
        : durationTicks(durationTicksValue), moveSpeed(moveSpeedValue),
          distortionTicks(distortionTicksValue), updateAttackIce(updateAttackIceValue),
          attackIce(attackIceValue), clearStun(clearStunValue), timerMode(timerModeValue) {}
};

class IStatusEffectService
{
public:
    virtual ~IStatusEffectService() {}

    virtual StatusEffectResult ApplyStun(EntityId target, int durationTicks) = 0;
    virtual StatusEffectResult ApplyPoison(EntityId target, int durationTicks, int damage) = 0;
    virtual StatusEffectResult ApplyHolyMind(EntityId target, int damageReduction, int durationTicks) = 0;
    virtual StatusEffectResult ApplyAssassinEye(EntityId target, std::uint32_t expiresAt,
                                                int subCritical) = 0;
    virtual StatusEffectResult ApplySlow(EntityId target, const SlowEffect& effect) = 0;
    virtual StatusEffectResult ApplyElementalSlow16Hz(EntityId target, int durationSeconds,
                                                       int moveSpeed, int signedIceResistance) = 0;
    virtual StatusEffectResult ClearStatus(EntityId target, StatusEffectKind kind) = 0;
    virtual StatusEffectResult ApplyHeavenlyLight(EntityId target, int attackRateReduction,
                                                  int durationTicks) = 0;
};
}
}
