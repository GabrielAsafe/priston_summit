#include "gameplay/skills/v2/SkillAnimationCommands.h"

#include <cassert>

namespace
{
class FakeSkillAnimationCommands final : public gameplay::skills::ISkillAnimationCommands
{
public:
    gameplay::skills::SkillAnimationLoopResult ConfigureLoop(gameplay::EntityId caster, int loopSpeed, int loopCount) override
    {
        gameplay::skills::SkillAnimationLoopResult result;
        if(!caster.Valid() || loopSpeed <= 0 || loopCount < 0)
            return result;
        result.code = gameplay::skills::SkillAnimationCommandCode::Ok;
        result.loopSpeed = loopSpeed;
        result.eventFrame2 = 210;
        result.configured = true;
        return result;
    }

    gameplay::skills::SkillAnimationLoopResult ClearLoop(gameplay::EntityId caster) override
    {
        gameplay::skills::SkillAnimationLoopResult result;
        if(!caster.Valid())
            return result;
        result.code = gameplay::skills::SkillAnimationCommandCode::Ok;
        result.configured = true;
        return result;
    }
};
}

int main()
{
    FakeSkillAnimationCommands service;
    const gameplay::skills::SkillAnimationLoopResult configured =
        service.ConfigureLoop(gameplay::EntityId(77u), 64, 1);
    assert(configured.Ok());
    assert(configured.loopSpeed == 64);
    assert(configured.eventFrame2 == 210);
    assert(service.ClearLoop(gameplay::EntityId(77u)).Ok());
    assert(!service.ConfigureLoop(gameplay::EntityId(), 64, 1).Ok());
    return 0;
}
