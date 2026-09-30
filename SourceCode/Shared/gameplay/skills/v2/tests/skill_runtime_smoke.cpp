#include "gameplay/skills/v2/runtime/SkillRuntime.h"

#include <cassert>

namespace
{
class FakeSkillRuntime final : public gameplay::skills::ISkillRuntime
{
public:
    gameplay::skills::SkillRuntimeResult Begin(const gameplay::skills::SkillRuntimeRequest& request) override
    {
        gameplay::skills::SkillRuntimeResult result;
        if(!request.caster.Valid() || request.skill == 0u || request.point < 1 || request.point > 10)
            return result;
        result.code = gameplay::skills::SkillRuntimeCode::Ok;
        result.playCode = 0x1234u | (static_cast<unsigned int>(request.visualParam & 0xF) << 12);
        result.motionState = 7u;
        result.desiredMotionSelected = true;
        return result;
    }
};
}

int main()
{
    FakeSkillRuntime runtime;
    const gameplay::skills::SkillRuntimeResult started = runtime.Begin(
        gameplay::skills::SkillRuntimeRequest(gameplay::EntityId(1u), 0x100u, 5, 3));
    assert(started.Ok());
    assert(started.motionState == 7u);
    assert((started.playCode & (3u << 12)) == (3u << 12));

    const gameplay::skills::SkillRuntimeResult rejected = runtime.Begin(
        gameplay::skills::SkillRuntimeRequest());
    assert(!rejected.Ok());
    return 0;
}
