#pragma once

#include "gameplay/skills/v2/SkillAnimationCommands.h"

namespace gameplay
{
namespace skills
{
namespace legacy
{
class LegacySkillAnimationAdapter final : public ISkillAnimationCommands
{
public:
    SkillAnimationLoopResult ConfigureLoop(EntityId caster, int loopSpeed, int loopCount) override;
    SkillAnimationLoopResult ClearLoop(EntityId caster) override;
};

LegacySkillAnimationAdapter& ServerLegacySkillAnimationAdapter();
}
}
}
