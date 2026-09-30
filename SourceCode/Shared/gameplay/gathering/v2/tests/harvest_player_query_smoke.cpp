#include <cassert>

#include "gameplay/gathering/v2/HarvestPlayerQuery.h"

namespace
{
class FakeHarvestPlayerQuery final : public gameplay::gathering::IHarvestPlayerQuery
{
public:
    bool TryGet(gameplay::PlayerId player, gameplay::gathering::HarvestPlayerSnapshot& out) const override
    {
        if (!player.Valid())
            return false;
        out.player = player;
        out.playerName = "harvester";
        out.area = 7;
        out.x = 100;
        out.y = 20;
        out.z = -30;
        return true;
    }
};
}

int main()
{
    FakeHarvestPlayerQuery query;
    gameplay::gathering::HarvestPlayerSnapshot snapshot;
    assert(!query.TryGet(gameplay::PlayerId(), snapshot));
    assert(query.TryGet(gameplay::PlayerId(42), snapshot));
    assert(snapshot.player == gameplay::PlayerId(42));
    assert(snapshot.playerName == "harvester");
    assert(snapshot.area == 7);
    assert(snapshot.x == 100);
    assert(snapshot.y == 20);
    assert(snapshot.z == -30);
    return 0;
}
