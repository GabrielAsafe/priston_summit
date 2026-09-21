#include <cassert>
#include <string>

#include "gameplay/progression/v2/ProgressionTriggerCoordinator.h"

int main()
{
    gameplay::progression::ProgressionTriggerCoordinator coordinator(4);

    assert(!coordinator.IsRunning("hero"));
    int runs = 0;
    bool nestedAccepted = false;
    const bool ok = coordinator.Request("hero", [&]()
    {
        ++runs;
        assert(coordinator.IsRunning("hero"));
        if (runs == 1)
        {
            nestedAccepted = coordinator.Request("hero", []() { return false; });
        }
        return true;
    });
    assert(ok);
    assert(nestedAccepted);
    assert(runs == 2);
    assert(!coordinator.IsRunning("hero"));

    int otherRuns = 0;
    assert(coordinator.Request("other", [&]() { ++otherRuns; return true; }));
    assert(otherRuns == 1);

    assert(!coordinator.Request("", []() { return true; }));

    gameplay::progression::ProgressionTriggerCoordinator bounded(2);
    int boundedRuns = 0;
    const bool boundedOk = bounded.Request("loop", [&]()
    {
        ++boundedRuns;
        bounded.Request("loop", []() { return true; });
        return true;
    });
    assert(!boundedOk);
    assert(boundedRuns == 2);

    return 0;
}
