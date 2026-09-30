#include "systems/gameplay/combat/v2/legacy/LegacyCombatBuffAdapter.h"

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
CombatBuffResult Result(CombatBuffCode code, bool applied = false)
{
    CombatBuffResult result;
    result.code = code;
    result.applied = applied;
    return result;
}

smCHAR* ResolveTarget(EntityId target, CombatBuffResult& failure)
{
    if(!target.Valid())
    {
        failure = Result(CombatBuffCode::InvalidRequest);
        return 0;
    }
    smCHAR* resolved = srFindCharFromSerial(static_cast<DWORD>(target.value));
    if(!resolved)
    {
        failure = Result(CombatBuffCode::TargetNotFound);
        return 0;
    }
    if(!resolved->Flag)
    {
        failure = Result(CombatBuffCode::TargetInactive);
        return 0;
    }
    return resolved;
}
}

CombatBuffResult LegacyCombatBuffAdapter::Apply(EntityId target, const CombatBuffDelta& delta)
{
    CombatBuffResult failure;
    smCHAR* resolved = ResolveTarget(target, failure);
    if(!resolved)
        return failure;

    resolved->smCharInfo.Attack_Damage[0] += delta.attackMin;
    resolved->smCharInfo.Attack_Damage[1] += delta.attackMax;
    resolved->smCharInfo.Attack_Rating += delta.attackRating;
    resolved->smCharInfo.Defence += delta.defence;
    resolved->smCharInfo.Attack_Speed += delta.attackSpeed;
    resolved->smCharInfo.Shooting_Range += delta.shootingRange;
    resolved->smCharInfo.Absorption += delta.absorption;
    resolved->smCharInfo.Chance_Block += delta.chanceBlock;
    resolved->smCharInfo.Life_Regen += delta.lifeRegen;
    resolved->smCharInfo.Mana_Regen += delta.manaRegen;
    for(int r = 0; r < 8; ++r)
        resolved->smCharInfo.Resistance[r] = static_cast<short>(
            static_cast<int>(resolved->smCharInfo.Resistance[r]) + delta.resistance[r]);
    resolved->smCharInfo.Life[1] = static_cast<short>(
        static_cast<int>(resolved->smCharInfo.Life[1]) + delta.maxLife);

    return Result(CombatBuffCode::Ok, true);
}

CombatBuffResult LegacyCombatBuffAdapter::Remove(EntityId target, const CombatBuffDelta& delta)
{
    CombatBuffResult failure;
    smCHAR* resolved = ResolveTarget(target, failure);
    if(!resolved)
        return failure;

    resolved->smCharInfo.Attack_Damage[0] -= delta.attackMin;
    resolved->smCharInfo.Attack_Damage[1] -= delta.attackMax;
    resolved->smCharInfo.Attack_Rating -= delta.attackRating;
    resolved->smCharInfo.Defence -= delta.defence;
    resolved->smCharInfo.Attack_Speed -= delta.attackSpeed;
    resolved->smCharInfo.Shooting_Range -= delta.shootingRange;
    resolved->smCharInfo.Absorption -= delta.absorption;
    resolved->smCharInfo.Chance_Block -= delta.chanceBlock;
    resolved->smCharInfo.Life_Regen -= delta.lifeRegen;
    resolved->smCharInfo.Mana_Regen -= delta.manaRegen;
    for(int r = 0; r < 8; ++r)
        resolved->smCharInfo.Resistance[r] = static_cast<short>(
            static_cast<int>(resolved->smCharInfo.Resistance[r]) - delta.resistance[r]);
    resolved->smCharInfo.Life[1] = static_cast<short>(std::max(1,
        static_cast<int>(resolved->smCharInfo.Life[1]) - delta.maxLife));
    if(resolved->smCharInfo.Life[0] > resolved->smCharInfo.Life[1])
        resolved->smCharInfo.Life[0] = resolved->smCharInfo.Life[1];

    if(resolved->smCharInfo.Attack_Damage[0] < 1)
        resolved->smCharInfo.Attack_Damage[0] = 1;
    if(resolved->smCharInfo.Attack_Damage[1] < resolved->smCharInfo.Attack_Damage[0])
        resolved->smCharInfo.Attack_Damage[1] = resolved->smCharInfo.Attack_Damage[0];
    if(resolved->smCharInfo.Attack_Rating < 0) resolved->smCharInfo.Attack_Rating = 0;
    if(resolved->smCharInfo.Defence < 0) resolved->smCharInfo.Defence = 0;
    if(resolved->smCharInfo.Attack_Speed < 1) resolved->smCharInfo.Attack_Speed = 1;
    if(resolved->smCharInfo.Shooting_Range < 0) resolved->smCharInfo.Shooting_Range = 0;
    if(resolved->smCharInfo.Absorption < 0) resolved->smCharInfo.Absorption = 0;
    if(resolved->smCharInfo.Chance_Block < 0) resolved->smCharInfo.Chance_Block = 0;
    if(resolved->smCharInfo.Life_Regen < 0.0f) resolved->smCharInfo.Life_Regen = 0.0f;
    if(resolved->smCharInfo.Mana_Regen < 0.0f) resolved->smCharInfo.Mana_Regen = 0.0f;

    return Result(CombatBuffCode::Ok, true);
}

LegacyCombatBuffAdapter& ServerLegacyCombatBuffAdapter()
{
    static LegacyCombatBuffAdapter adapter;
    return adapter;
}
}
}
}
