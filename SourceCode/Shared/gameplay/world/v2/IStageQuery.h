#pragma once

#include <cstdint>

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace world
{
struct StageSnapshot
{
    MapId map;
    bool ready;

    StageSnapshot() : map(), ready(false) {}
    StageSnapshot(MapId mapValue, bool readyValue) : map(mapValue), ready(readyValue) {}
};

class IStageQuery
{
public:
    virtual ~IStageQuery() {}
    virtual bool TryGetStage(MapId map, StageSnapshot& out) const = 0;
    virtual bool IsReady(MapId map) const = 0;
    virtual bool TryGetGroundHeight(MapId map,
                                    std::int32_t x,
                                    std::int32_t z,
                                    std::int32_t& y) const = 0;
};
}
}
