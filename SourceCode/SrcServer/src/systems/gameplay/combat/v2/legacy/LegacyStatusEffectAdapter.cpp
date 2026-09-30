#include "systems/gameplay/combat/v2/legacy/LegacyStatusEffectAdapter.h"

#include <algorithm>

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"

namespace gameplay
{
namespace combat
{
namespace legacy
{
namespace
{
StatusEffectResult Result(StatusEffectCode code, bool applied = false)
{
    StatusEffectResult result;
    result.code = code;
    result.applied = applied;
    return result;
}

smCHAR* ResolveTarget(EntityId target, StatusEffectResult& failure)
{
    if(!target.Valid())
    {
        failure = Result(StatusEffectCode::InvalidRequest);
        return 0;
    }
    smCHAR* resolved = srFindCharFromSerial(static_cast<DWORD>(target.value));
    if(!resolved)
    {
        failure = Result(StatusEffectCode::TargetNotFound);
        return 0;
    }
    if(!resolved->Flag)
    {
        failure = Result(StatusEffectCode::TargetInactive);
        return 0;
    }
    return resolved;
}
}

StatusEffectResult LegacyStatusEffectAdapter::ApplyStun(EntityId target, int durationTicks)
{
    if(durationTicks < 0)
        return Result(StatusEffectCode::InvalidRequest);
    StatusEffectResult failure;
    smCHAR* resolved = ResolveTarget(target, failure);
    if(!resolved)
        return failure;
    resolved->PlayStunCount = durationTicks;
    return Result(StatusEffectCode::Ok, true);
}

StatusEffectResult LegacyStatusEffectAdapter::ApplyPoison(EntityId target, int durationTicks, int damage)
{
    if(durationTicks < 0 || damage < 0)
        return Result(StatusEffectCode::InvalidRequest);
    StatusEffectResult failure;
    smCHAR* resolved = ResolveTarget(target, failure);
    if(!resolved)
        return failure;
    resolved->PlayPoison[1] = static_cast<short>(durationTicks);
    resolved->PlayPoison[0] = static_cast<short>(damage);
    return Result(StatusEffectCode::Ok, true);
}

StatusEffectResult LegacyStatusEffectAdapter::ApplyHolyMind(EntityId target, int damageReduction,
                                                             int durationTicks)
{
    if(durationTicks < 0)
        return Result(StatusEffectCode::InvalidRequest);
    StatusEffectResult failure;
    smCHAR* resolved = ResolveTarget(target, failure);
    if(!resolved)
        return failure;
    resolved->PlayHolyMind[0] = static_cast<short>(damageReduction);
    resolved->PlayHolyMind[1] = static_cast<short>(durationTicks);
    return Result(StatusEffectCode::Ok, true);
}

StatusEffectResult LegacyStatusEffectAdapter::ApplyAssassinEye(EntityId target, std::uint32_t expiresAt,
                                                                int subCritical)
{
    if(expiresAt == 0u)
        return Result(StatusEffectCode::InvalidRequest);
    StatusEffectResult failure;
    smCHAR* resolved = ResolveTarget(target, failure);
    if(!resolved)
        return failure;
    resolved->dwAssassinEyeTime = static_cast<DWORD>(expiresAt);
    resolved->AssassinEyeParam = subCritical;
    return Result(StatusEffectCode::Ok, true);
}

StatusEffectResult LegacyStatusEffectAdapter::ApplySlow(EntityId target, const SlowEffect& effect)
{
    if(effect.durationTicks < 0 || effect.distortionTicks < 0)
        return Result(StatusEffectCode::InvalidRequest);
    StatusEffectResult failure;
    smCHAR* resolved = ResolveTarget(target, failure);
    if(!resolved)
        return failure;
    resolved->PlaySlowCount = effect.durationTicks;
    resolved->PlaySlowSpeed = effect.moveSpeed;
    resolved->PlayDistortion = effect.distortionTicks;
    if(effect.clearStun)
        resolved->PlayStunCount = 0;
    if(effect.updateAttackIce)
        resolved->AttackIce = effect.attackIce ? 1 : 0;
    return Result(StatusEffectCode::Ok, true);
}

StatusEffectResult LegacyStatusEffectAdapter::ApplyElementalSlow16Hz(EntityId target, int durationSeconds,
                                                                     int moveSpeed, int signedIceResistance)
{
    if(durationSeconds < 0)
        return Result(StatusEffectCode::InvalidRequest);
    StatusEffectResult failure;
    smCHAR* resolved = ResolveTarget(target, failure);
    if(!resolved)
        return failure;

    const int resistance = std::max(-100, std::min(100, signedIceResistance));
    int durationTicks = durationSeconds * 16;
    durationTicks -= (durationTicks * resistance) / 100;
    resolved->PlaySlowCount = std::max(0, durationTicks);
    resolved->PlaySlowSpeed = moveSpeed;
    resolved->PlayDistortion = 0;
    return Result(StatusEffectCode::Ok, true);
}

StatusEffectResult LegacyStatusEffectAdapter::ClearStatus(EntityId target, StatusEffectKind kind)
{
    StatusEffectResult failure;
    smCHAR* resolved = ResolveTarget(target, failure);
    if(!resolved)
        return failure;

    switch(kind)
    {
        case StatusEffectKind::Stun:
            resolved->PlayStunCount = 0;
            return Result(StatusEffectCode::Ok, true);
    }
    return Result(StatusEffectCode::InvalidRequest);
}

StatusEffectResult LegacyStatusEffectAdapter::ApplyHeavenlyLight(EntityId target, int attackRateReduction,
                                                                  int durationTicks)
{
    if(durationTicks < 0)
        return Result(StatusEffectCode::InvalidRequest);
    StatusEffectResult failure;
    smCHAR* resolved = ResolveTarget(target, failure);
    if(!resolved)
        return failure;
    resolved->PlayHeavenlyLight[0] = static_cast<short>(attackRateReduction);
    resolved->PlayHeavenlyLight[1] = static_cast<short>(durationTicks);
    return Result(StatusEffectCode::Ok, true);
}

LegacyStatusEffectAdapter& ServerLegacyStatusEffectAdapter()
{
    static LegacyStatusEffectAdapter adapter;
    return adapter;
}
}
}
}
