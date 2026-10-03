#pragma once

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace skills
{
enum class SkillAnimationCommandCode
{
    Ok = 0,
    InvalidRequest,
    CasterNotFound,
    CasterInactive
};

struct SkillAnimationLoopResult
{
    SkillAnimationCommandCode code;
    int loopSpeed;
    int eventFrame2;
    bool configured;

    SkillAnimationLoopResult()
        : code(SkillAnimationCommandCode::InvalidRequest), loopSpeed(0), eventFrame2(0), configured(false) {}

    bool Ok() const { return code == SkillAnimationCommandCode::Ok && configured; }
};

class ISkillAnimationCommands
{
public:
    virtual ~ISkillAnimationCommands() {}
    virtual SkillAnimationLoopResult ConfigureLoop(EntityId caster, int loopSpeed, int loopCount) = 0;
    virtual SkillAnimationLoopResult ClearLoop(EntityId caster) = 0;
};
}
}
