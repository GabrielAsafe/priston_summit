#include "systems/gameplay/monsters/v2/legacy/LegacyMonsterAdapter.h"

#include <algorithm>

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"
#include "inventory/legacy/sinItem_legacy_server.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

namespace gameplay
{
namespace monsters
{
namespace legacy
{
namespace
{
FollowerConversionResult Reject(MonsterCommandCode code)
{
    FollowerConversionResult result;
    result.code = code;
    return result;
}
}

namespace
{
MonsterFamily ToMonsterFamily(int brood)
{
    switch(brood)
    {
        case smCHAR_MONSTER_NORMAL: return MonsterFamily::Normal;
        case smCHAR_MONSTER_UNDEAD: return MonsterFamily::Undead;
        case smCHAR_MONSTER_MUTANT: return MonsterFamily::Mutant;
        case smCHAR_MONSTER_DEMON: return MonsterFamily::Demon;
        case smCHAR_MONSTER_MECHANIC: return MonsterFamily::Mechanic;
        case smCHAR_MONSTER_USER: return MonsterFamily::PlayerFollower;
        default: return MonsterFamily::Unknown;
    }
}
}

bool LegacyMonsterAdapter::TryGetMonster(EntityId entity, MonsterSnapshot& out) const
{
    if(!entity.Valid())
        return false;

    smCHAR* target = srFindCharFromSerial(static_cast<DWORD>(entity.value));
    if(!target)
        return false;

    MonsterSnapshot snapshot;
    snapshot.entity = entity;
    snapshot.active = target->Flag != 0;
    snapshot.hostile = target->smCharInfo.State == smCHAR_STATE_ENEMY;
    snapshot.controlled = target->lpMasterPlayInfo != 0 || target->smCharInfo.Brood == smCHAR_MONSTER_USER;
    snapshot.family = ToMonsterFamily(target->smCharInfo.Brood);
    snapshot.level = target->smCharInfo.Level;
    snapshot.life = target->smCharInfo.Life[0];
    snapshot.maxLife = target->smCharInfo.Life[1];
    snapshot.bionicResistance = target->smCharInfo.Resistance[sITEMINFO_BIONIC];
    snapshot.iceResistance = target->smCharInfo.Resistance[sITEMINFO_ICE];
    out = snapshot;
    return true;
}

FollowerConversionResult LegacyMonsterAdapter::ConvertToFollower(const FollowerConversionRequest& request)
{
    if(!request.target.Valid() || !request.owner.Valid() || request.durationTicks <= 0 || request.lifePercent <= 0)
        return Reject(MonsterCommandCode::InvalidRequest);

    rsPLAYINFO* owner = player::legacy::ServerLegacyOnlinePlayerRegistry().Resolve(request.owner);
    if(!owner)
        return Reject(MonsterCommandCode::OwnerNotFound);

    smCHAR* target = srFindCharFromSerial(static_cast<DWORD>(request.target.value));
    if(!target)
        return Reject(MonsterCommandCode::TargetNotFound);
    if(!target->Flag)
        return Reject(MonsterCommandCode::TargetInactive);
    if(target->smCharInfo.Life[0] <= 0)
        return Reject(MonsterCommandCode::TargetDead);
    if(target->smCharInfo.Brood == smCHAR_MONSTER_USER || target->lpMasterPlayInfo)
        return Reject(MonsterCommandCode::InvalidState);

    FollowerConversionResult result;
    result.previousBrood = target->smCharInfo.Brood;
    result.previousLife = target->smCharInfo.Life[0];
    result.durationTicks = request.durationTicks;
    result.convertedLife = std::max(1, std::min(32767,
        (result.previousLife * request.lifePercent) / 100));

    target->lpMasterPlayInfo = owner;
    target->smMonsterInfo.dwAutoCharCode = 0;
    target->smCharInfo.bUpdateInfo[0]++;
    target->PlayHolyIncantation[0] = static_cast<short>(result.previousBrood);
    target->PlayHolyIncantation[1] = static_cast<short>(request.durationTicks);
    target->smCharInfo.Life[0] = target->smCharInfo.Life[1] = static_cast<short>(result.convertedLife);
    target->smCharInfo.Next_Exp = owner->dwObjectSerial;
    target->smCharInfo.Brood = smCHAR_MONSTER_USER;
    target->lpTargetChar = 0;
    target->lpTargetPlayInfo = 0;

    result.code = MonsterCommandCode::Ok;
    result.converted = true;
    return result;
}

LegacyMonsterAdapter& ServerLegacyMonsterAdapter()
{
    static LegacyMonsterAdapter adapter;
    return adapter;
}
}
}
}
