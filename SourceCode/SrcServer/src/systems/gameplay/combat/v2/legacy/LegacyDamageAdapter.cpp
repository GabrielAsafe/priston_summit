#include "systems/gameplay/combat/v2/legacy/LegacyDamageAdapter.h"

#include <algorithm>

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

namespace gameplay
{
namespace combat
{
namespace legacy
{
namespace
{
DamageResult Reject(const DamageRequest& request, DamageRejectReason reason, int remainingLife = 0)
{
    DamageResult result;
    result.rejectReason = reason;
    result.rawDamage = request.rawDamage;
    result.remainingLife = remainingLife;
    return result;
}
}

DamageResult LegacyDamageAdapter::Apply(const DamageRequest& request)
{
    if(!request.source.Valid() || !request.owner.Valid() || !request.target.Valid() || request.rawDamage <= 0)
        return Reject(request, DamageRejectReason::InvalidRequest);

    smCHAR* caster = srFindCharFromSerial(static_cast<DWORD>(request.source.value));
    if(!caster || !caster->Flag)
        return Reject(request, DamageRejectReason::SourceNotFound);

    rsPLAYINFO* owner = player::legacy::ServerLegacyOnlinePlayerRegistry().Resolve(request.owner);
    if(!owner)
        return Reject(request, DamageRejectReason::OwnerNotFound);

    smCHAR* target = srFindCharFromSerial(static_cast<DWORD>(request.target.value));
    if(!target)
        return Reject(request, DamageRejectReason::TargetNotFound);
    if(!target->Flag)
        return Reject(request, DamageRejectReason::TargetInactive, target->smCharInfo.Life[0]);
    if(target->smCharInfo.Life[0] <= 0)
        return Reject(request, DamageRejectReason::TargetDead, target->smCharInfo.Life[0]);
    if(target->smCharInfo.State != smCHAR_STATE_ENEMY)
        return Reject(request, DamageRejectReason::TargetNotEnemy, target->smCharInfo.Life[0]);
    if(target->smCharInfo.Brood == smCHAR_MONSTER_USER)
        return Reject(request, DamageRejectReason::TargetIsPlayerMonster, target->smCharInfo.Life[0]);
    if(target->lpMasterPlayInfo)
        return Reject(request, DamageRejectReason::TargetIsOwnedMonster, target->smCharInfo.Life[0]);

    int damage = request.rawDamage;
    damage -= (damage * target->smCharInfo.Absorption) / 100;
    if(damage < 1)
        damage = 1;

    target->smCharInfo.Life[0] -= damage;

    DamageResult result;
    result.rejectReason = DamageRejectReason::None;
    result.rawDamage = request.rawDamage;
    result.finalDamage = damage;
    result.applied = true;

    if(target->smCharInfo.Life[0] <= 0)
    {
        if(target->UseObject_VirtualLife && target->sObject_VirtualLife[0] > 0)
        {
            target->smCharInfo.Life[0] = std::max(1, target->smCharInfo.Life[1] / 100);
            result.virtualLifeTriggered = true;
        }
        else
        {
            target->smCharInfo.Life[0] = 0;
            target->SetMotionFromCode(CHRMOTION_STATE_DEAD);
            if(owner->dwObjectSerial && owner->dwObjectSerial == caster->smCharInfo.Next_Exp)
                target->smMonsterInfo.GetExp >>= 1;
            target->lpExt2 = owner;
            target->lpExt3 = caster;
            result.killed = true;
        }
    }
    else if(target->MotionInfo && target->MotionInfo->State < 0x100)
    {
        target->SetMotionFromCode(CHRMOTION_STATE_DAMAGE);
    }

    result.remainingLife = target->smCharInfo.Life[0];
    return result;
}

LegacyDamageAdapter& ServerLegacyDamageAdapter()
{
    static LegacyDamageAdapter adapter;
    return adapter;
}
}
}
}
