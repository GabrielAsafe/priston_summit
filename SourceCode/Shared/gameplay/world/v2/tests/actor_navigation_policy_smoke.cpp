#include "gameplay/world/v2/ActorNavigationPolicy.h"
#include <cassert>

int main()
{
    using gameplay::world::ActorNavigationPolicy;
    gameplay::world::ActorAutoMovePlan autoMove = ActorNavigationPolicy::PlanAutoMove(4, 0);
    assert(autoMove.walkStepCount == 0);
    assert(autoMove.runStepCount == 0);
    assert(!autoMove.preferRun);

    autoMove = ActorNavigationPolicy::PlanAutoMove(6, (64 * 8 * 64 * 8) - 1);
    assert(autoMove.walkStepCount == 1);
    assert(autoMove.runStepCount == 4);
    assert(autoMove.preferRun);

    autoMove = ActorNavigationPolicy::PlanAutoMove(10, (64 * 8 * 64 * 8));
    assert(autoMove.walkStepCount == 5);
    assert(autoMove.runStepCount == 20);
    assert(!autoMove.preferRun);
    return 0;
}
