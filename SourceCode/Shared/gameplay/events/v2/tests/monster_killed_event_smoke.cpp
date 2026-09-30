#include <cassert>

#include "systems/gameplay/events/v2/MonsterKilledEvent.h"

int main()
{
    gameplay::events::MonsterKilledEvent event(
        gameplay::PlayerId(7), gameplay::EntityId(99), true, true,
        "Player", "Boss", 1234u, 5);
    assert(event.player == gameplay::PlayerId(7));
    assert(event.monster == gameplay::EntityId(99));
    assert(event.partyCredit);
    assert(event.monsterBoss);
    assert(event.playerName == "Player");
    assert(event.monsterName == "Boss");
    assert(event.monsterCode == 1234u);
    assert(event.area == 5);
    return 0;
}
