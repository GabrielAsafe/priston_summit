#include <cassert>

#include "gameplay/events/v2/GameplayEventHub.h"
#include "gameplay/events/v2/MonsterDefeatedEvent.h"

int main()
{
    gameplay::events::GameplayEventHub hub;
    int seen = 0;
    gameplay::EntityId captured;
    hub.Subscribe<gameplay::events::MonsterDefeatedEvent>(
        [&](const gameplay::events::MonsterDefeatedEvent& event)
        {
            ++seen;
            captured = event.monster;
        });

    hub.Publish(gameplay::events::MonsterDefeatedEvent(gameplay::EntityId(77u)));
    assert(seen == 1);
    assert(captured == gameplay::EntityId(77u));
    return 0;
}
