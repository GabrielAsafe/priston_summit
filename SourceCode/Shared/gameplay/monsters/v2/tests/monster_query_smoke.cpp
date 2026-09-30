#include <cassert>

#include "gameplay/monsters/v2/MonsterQuery.h"

namespace
{
class FakeMonsterQuery final : public gameplay::monsters::IMonsterQuery
{
public:
    bool TryGetMonster(gameplay::EntityId entity, gameplay::monsters::MonsterSnapshot& out) const override
    {
        if (!entity.Valid())
            return false;
        out.entity = entity;
        out.active = true;
        out.hostile = true;
        out.family = gameplay::monsters::MonsterFamily::Undead;
        out.level = 77;
        out.life = 1000;
        out.maxLife = 1500;
        out.bionicResistance = -20;
        return true;
    }
};
}

int main()
{
    FakeMonsterQuery query;
    gameplay::monsters::MonsterSnapshot snapshot;
    assert(query.TryGetMonster(gameplay::EntityId(9u), snapshot));
    assert(snapshot.family == gameplay::monsters::MonsterFamily::Undead);
    assert(snapshot.bionicResistance == -20);
    assert(snapshot.life == 1000);
    assert(!query.TryGetMonster(gameplay::EntityId(), snapshot));
    return 0;
}
