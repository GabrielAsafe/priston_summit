#pragma once

namespace gameplay
{
namespace world
{
struct ActorAutoMovePlan
{
    int walkStepCount;
    int runStepCount;
    bool preferRun;

    ActorAutoMovePlan(int walkSteps = 0, int runSteps = 0, bool preferRunValue = false)
        : walkStepCount(walkSteps), runStepCount(runSteps), preferRun(preferRunValue) {}
};

class ActorNavigationPolicy
{
public:
    static ActorAutoMovePlan PlanAutoMove(int intelligence, int distanceSquared)
    {
        int walkSteps = intelligence - 5;
        if(walkSteps < 0)
            walkSteps = 0;

        const bool preferRun = intelligence >= 6 && distanceSquared < (64 * 8 * 64 * 8);
        return ActorAutoMovePlan(walkSteps, walkSteps << 2, preferRun);
    }
};
}
}
