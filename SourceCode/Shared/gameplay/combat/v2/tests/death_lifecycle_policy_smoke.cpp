#include "gameplay/combat/v2/DeathLifecyclePolicy.h"
#include <cassert>

int main()
{
    using gameplay::combat::DeathLifecyclePolicy;
    assert(!DeathLifecyclePolicy::ShouldResolveChangeJobArenaDeath(false, false));
    assert(!DeathLifecyclePolicy::ShouldResolveChangeJobArenaDeath(true, false));
    assert(!DeathLifecyclePolicy::ShouldResolveChangeJobArenaDeath(false, true));
    assert(DeathLifecyclePolicy::ShouldResolveChangeJobArenaDeath(true, true));
    return 0;
}
