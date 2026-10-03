#include "systems/gameplay/skills/v2/legacy/LegacySkillAnimationAdapter.h"

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"

namespace gameplay
{
namespace skills
{
namespace legacy
{
namespace
{
SkillAnimationLoopResult Reject(SkillAnimationCommandCode code)
{
    SkillAnimationLoopResult result;
    result.code = code;
    return result;
}

smCHAR* ResolveCaster(EntityId caster, SkillAnimationLoopResult& failure)
{
    if(!caster.Valid())
    {
        failure = Reject(SkillAnimationCommandCode::InvalidRequest);
        return 0;
    }
    smCHAR* resolved = srFindCharFromSerial(static_cast<DWORD>(caster.value));
    if(!resolved)
    {
        failure = Reject(SkillAnimationCommandCode::CasterNotFound);
        return 0;
    }
    if(!resolved->Flag)
    {
        failure = Reject(SkillAnimationCommandCode::CasterInactive);
        return 0;
    }
    return resolved;
}
}

SkillAnimationLoopResult LegacySkillAnimationAdapter::ConfigureLoop(EntityId caster, int loopSpeed, int loopCount)
{
    if(loopSpeed <= 0 || loopCount < 0)
        return Reject(SkillAnimationCommandCode::InvalidRequest);

    SkillAnimationLoopResult failure;
    smCHAR* resolved = ResolveCaster(caster, failure);
    if(!resolved)
        return failure;

    resolved->MotionLoopSpeed = loopSpeed;
    resolved->MotionLoop = loopCount;

    SkillAnimationLoopResult result;
    result.code = SkillAnimationCommandCode::Ok;
    result.loopSpeed = resolved->MotionLoopSpeed;
    result.eventFrame2 = resolved->MotionInfo ? resolved->MotionInfo->EventFrame[2] : 0;
    result.configured = true;
    return result;
}

SkillAnimationLoopResult LegacySkillAnimationAdapter::ClearLoop(EntityId caster)
{
    SkillAnimationLoopResult failure;
    smCHAR* resolved = ResolveCaster(caster, failure);
    if(!resolved)
        return failure;

    resolved->MotionLoop = 0;

    SkillAnimationLoopResult result;
    result.code = SkillAnimationCommandCode::Ok;
    result.loopSpeed = resolved->MotionLoopSpeed;
    result.eventFrame2 = resolved->MotionInfo ? resolved->MotionInfo->EventFrame[2] : 0;
    result.configured = true;
    return result;
}

LegacySkillAnimationAdapter& ServerLegacySkillAnimationAdapter()
{
    static LegacySkillAnimationAdapter adapter;
    return adapter;
}
}
}
}
