#include <cassert>
#include <string>

#include "gameplay/events/v2/GameplayEventHub.h"
#include "gameplay/events/v2/WorldEventLifecycleEvents.h"

int main()
{
    gameplay::events::GameplayEventHub hub;
    int seen = 0;
    gameplay::events::EventScoreChanged captured;
    hub.Subscribe<gameplay::events::EventScoreChanged>(
        [&](const gameplay::events::EventScoreChanged& event)
        {
            ++seen;
            captured = event;
        });
    hub.Publish(gameplay::events::EventScoreChanged(
        "bellatra", gameplay::PlayerId(7u), 5000, 7500, 2, 12000));
    assert(seen == 1);
    assert(captured.eventKey == "bellatra");
    assert(captured.player == gameplay::PlayerId(7u));
    assert(captured.delta == 5000);
    assert(captured.playerScore == 7500);
    assert(captured.team == 2);
    assert(captured.teamScore == 12000);
    return 0;
}
