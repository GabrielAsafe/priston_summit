#include <cassert>

#include "systems/gameplay/events/v2/MonsterKilledEvent.h"
#include "systems/gameplay/hunting/v2/HuntingService.h"

int main()
{
    gameplay::hunting::HuntingService service;

    gameplay::events::MonsterKilledEvent solo(
        0, 0, false, "Archer", "Hopy", 101u, 2);
    gameplay::events::MonsterKilledEvent party(
        0, 0, true, "Archer", "Hopy", 101u, 2);
    gameplay::events::MonsterKilledEvent other(
        0, 0, false, "Archer", "Goblin", 202u, 4);

    service.OnMonsterKilled(solo);
    service.OnMonsterKilled(party);
    service.OnMonsterKilled(other);

    gameplay::hunting::HuntingRuntimeProgress progress;
    assert(service.GetProgress("Archer", progress));
    assert(progress.totalKills == 3);
    assert(progress.soloKills == 2);
    assert(progress.partyKills == 1);
    assert(progress.lastArea == 4);
    assert(progress.lastMonsterName == "Goblin");
    assert(service.GetMonsterKills("Archer", 101u) == 2);
    assert(service.GetMonsterKills("Archer", 202u) == 1);
    assert(service.GetMonsterKills("Missing", 101u) == 0);

    service.ClearPlayer("Archer");
    assert(!service.GetProgress("Archer", progress));
    return 0;
}
