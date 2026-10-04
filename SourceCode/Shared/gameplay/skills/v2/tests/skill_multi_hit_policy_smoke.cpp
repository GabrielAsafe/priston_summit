#include "gameplay/skills/v2/SkillMultiHitPolicy.h"
#include <cassert>

int main()
{
    using gameplay::skills::SkillMultiHitPolicy;

    assert(SkillMultiHitPolicy::IsWithinHitWindow(0, 2));
    assert(SkillMultiHitPolicy::IsWithinHitWindow(1, 2));
    assert(!SkillMultiHitPolicy::IsWithinHitWindow(2, 2));

    assert(SkillMultiHitPolicy::IsWithinHitWindow(0, 3));
    assert(SkillMultiHitPolicy::IsWithinHitWindow(2, 3));
    assert(!SkillMultiHitPolicy::IsWithinHitWindow(3, 3));
    assert(!SkillMultiHitPolicy::IsWithinHitWindow(4, 3));

    assert(SkillMultiHitPolicy::IsWithinHitWindow(-1, 0));
    assert(!SkillMultiHitPolicy::IsWithinHitWindow(0, 0));
    return 0;
}
