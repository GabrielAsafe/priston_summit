#include <cassert>
#include <map>

#include "gameplay/world/v2/IWorldQuery.h"

namespace
{
class FakeWorld final : public gameplay::world::IWorldQuery
{
public:
    std::map<std::uint32_t, gameplay::WorldPosition> positions;
    bool TryGetPosition(gameplay::EntityId entity, gameplay::WorldPosition& out) const override
    {
        std::map<std::uint32_t, gameplay::WorldPosition>::const_iterator it=positions.find(entity.value);
        if(it==positions.end()) return false;
        out=it->second; return true;
    }
    bool IsValidPosition(const gameplay::WorldPosition& p) const override { return p.map.Valid(); }
    bool TryGetGroundHeight(gameplay::MapId map, std::int32_t, std::int32_t, std::int32_t& y) const override
    { if(!map.Valid()) return false; y=123; return true; }
    bool IsSameArea(gameplay::EntityId a, gameplay::EntityId b) const override
    { gameplay::WorldPosition pa,pb; return TryGetPosition(a,pa)&&TryGetPosition(b,pb)&&pa.map==pb.map; }
};
}

int main()
{
    assert(gameplay::MapId(0u).Valid());
    assert(!gameplay::MapId().Valid());
    FakeWorld world;
    world.positions[1u]=gameplay::WorldPosition(gameplay::MapId(0u), 256, 512, 768);
    world.positions[2u]=gameplay::WorldPosition(gameplay::MapId(0u), 512, 512, 768);
    gameplay::WorldPosition p;
    assert(world.TryGetPosition(gameplay::EntityId(1u),p));
    assert(p.map.value==0u);
    assert(gameplay::WorldFixedToGrid(p.x)==1);
    std::int32_t y=0;
    assert(world.TryGetGroundHeight(gameplay::MapId(0u), p.x, p.z, y) && y==123);
    assert(world.IsSameArea(gameplay::EntityId(1u),gameplay::EntityId(2u)));
    return 0;
}
