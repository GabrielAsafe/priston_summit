#include "systems/gameplay/monsters/v2/legacy/LegacySummonedActorAdapter.h"

#include <algorithm>

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"

namespace gameplay
{
namespace monsters
{
namespace legacy
{
SummonedActorConfigureResult LegacySummonedActorAdapter::Configure(
    const SummonedActorConfigureRequest& request)
{
    SummonedActorConfigureResult result;
    result.actor = request.actor;
    if(!request.actor.Valid() || request.profile.life <= 0 ||
       request.profile.attackDamageMax < request.profile.attackDamageMin)
        return result;

    smCHAR* actor = srFindCharFromSerial(static_cast<DWORD>(request.actor.value));
    if(!actor)
    {
        result.code = SummonedActorCommandCode::NotFound;
        return result;
    }
    if(!actor->Flag)
    {
        result.code = SummonedActorCommandCode::Inactive;
        return result;
    }

    actor->dwUpdateCharInfoTime = static_cast<DWORD>(request.profile.expiresAtMs);
    actor->smCharInfo.Attack_Damage[0] = request.profile.attackDamageMin;
    actor->smCharInfo.Attack_Damage[1] = request.profile.attackDamageMax;
    actor->smCharInfo.Attack_Rating = request.profile.attackRating;
    if(request.profile.overrideDefence)
        actor->smCharInfo.Defence = request.profile.defence;

    result.appliedLife = std::min(32767, std::max(1, request.profile.life));
    actor->smCharInfo.Life[0] = static_cast<short>(result.appliedLife);
    actor->smCharInfo.Life[1] = static_cast<short>(result.appliedLife);
    actor->sMoveStepCount[0] = static_cast<short>(request.profile.moveStepPrimary);
    actor->sMoveStepCount[1] = static_cast<short>(request.profile.moveStepSecondary);

    if(request.profile.overridePlayClass)
        actor->smCharInfo.wPlayClass[1] = static_cast<WORD>(request.profile.playClassPhase);
    if(request.profile.overrideVisualSkill)
    {
        actor->HoSkillCode = request.profile.visualSkillCode;
        actor->HoSkillMode = request.profile.visualSkillMode;
    }

    result.code = SummonedActorCommandCode::Ok;
    return result;
}

LegacySummonedActorAdapter& ServerLegacySummonedActorAdapter()
{
    static LegacySummonedActorAdapter adapter;
    return adapter;
}
}
}
}
