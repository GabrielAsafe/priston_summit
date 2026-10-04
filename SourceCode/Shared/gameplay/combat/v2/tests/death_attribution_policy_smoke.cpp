#include "gameplay/combat/v2/DeathAttributionPolicy.h"
#include <cassert>

int main()
{
    using gameplay::combat::DeathAttributionPolicy;

    assert(DeathAttributionPolicy::ShouldApplyDeathExperiencePenalty(false, false));
    assert(!DeathAttributionPolicy::ShouldRecordPlayerKill(false, false));

    assert(!DeathAttributionPolicy::ShouldApplyDeathExperiencePenalty(true, false));
    assert(DeathAttributionPolicy::ShouldRecordPlayerKill(true, false));

    assert(!DeathAttributionPolicy::ShouldApplyDeathExperiencePenalty(false, true));
    assert(DeathAttributionPolicy::ShouldRecordPlayerKill(false, true));

    assert(!DeathAttributionPolicy::ShouldApplyDeathExperiencePenalty(true, true));
    assert(DeathAttributionPolicy::ShouldRecordPlayerKill(true, true));

    return 0;
}
