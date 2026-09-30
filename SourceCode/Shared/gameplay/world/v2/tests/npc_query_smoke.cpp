#include <cassert>

#include "gameplay/world/v2/INpcQuery.h"

namespace
{
class FakeNpcQuery final : public gameplay::world::INpcQuery
{
public:
    bool TryGetNpc(gameplay::EntityId entity, gameplay::world::NpcSnapshot& out) const override
    {
        if (!entity.Valid())
            return false;
        out = gameplay::world::NpcSnapshot(entity, 988u);
        return true;
    }
};
}

int main()
{
    FakeNpcQuery query;
    gameplay::world::NpcSnapshot npc;
    assert(query.TryGetNpc(gameplay::EntityId(7u), npc));
    assert(npc.entity.value == 7u);
    assert(npc.eventCode == 988u);
    assert(!query.TryGetNpc(gameplay::EntityId(), npc));
    return 0;
}
