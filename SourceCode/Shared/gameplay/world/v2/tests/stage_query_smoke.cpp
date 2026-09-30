#include <cassert>
#include <map>

#include "gameplay/world/v2/IStageQuery.h"

namespace
{
class FakeStage final : public gameplay::world::IStageQuery
{
public:
    std::map<std::uint32_t, bool> ready;
    bool TryGetStage(gameplay::MapId map, gameplay::world::StageSnapshot& out) const override
    {
        std::map<std::uint32_t,bool>::const_iterator it=ready.find(map.value);
        if(!map.Valid() || it==ready.end()) return false;
        out=gameplay::world::StageSnapshot(map,it->second); return true;
    }
    bool IsReady(gameplay::MapId map) const override
    { gameplay::world::StageSnapshot s; return TryGetStage(map,s)&&s.ready; }
    bool TryGetGroundHeight(gameplay::MapId map,std::int32_t,std::int32_t,std::int32_t& y) const override
    { if(!IsReady(map)) return false; y=4096; return true; }
};
}

int main()
{
    FakeStage stages;
    stages.ready[0u]=true;
    stages.ready[1u]=false;
    assert(stages.IsReady(gameplay::MapId(0u)));
    assert(!stages.IsReady(gameplay::MapId(1u)));
    assert(!stages.IsReady(gameplay::MapId(99u)));
    std::int32_t y=0;
    assert(stages.TryGetGroundHeight(gameplay::MapId(0u),100,200,y));
    assert(y==4096);
    assert(!stages.TryGetGroundHeight(gameplay::MapId(1u),100,200,y));
    return 0;
}
