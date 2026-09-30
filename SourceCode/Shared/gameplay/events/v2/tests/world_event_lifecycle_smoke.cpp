#include <cassert>
#include <string>

#include "gameplay/events/v2/GameplayEventHub.h"
#include "gameplay/events/v2/WorldEventLifecycleEvents.h"

int main()
{
    gameplay::events::GameplayEventHub hub;
    int started = 0;
    int ended = 0;
    bool completed = false;
    std::string key;

    hub.Subscribe<gameplay::events::WorldEventStarted>(
        [&](const gameplay::events::WorldEventStarted& event)
        {
            ++started;
            key = event.eventKey;
        });
    hub.Subscribe<gameplay::events::WorldEventEnded>(
        [&](const gameplay::events::WorldEventEnded& event)
        {
            ++ended;
            completed = event.completed;
        });

    hub.Publish(gameplay::events::WorldEventStarted("hells_gate"));
    hub.Publish(gameplay::events::WorldEventEnded("hells_gate", true));

    assert(started == 1);
    assert(ended == 1);
    assert(key == "hells_gate");
    assert(completed);
    return 0;
}
