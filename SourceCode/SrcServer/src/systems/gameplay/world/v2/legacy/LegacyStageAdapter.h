#pragma once

#include "gameplay/world/v2/IStageQuery.h"

namespace gameplay
{
namespace world
{
namespace legacy
{
class LegacyStageAdapter final : public IStageQuery
{
public:
    bool TryGetStage(MapId map, StageSnapshot& out) const override;
    bool IsReady(MapId map) const override;
    bool TryGetGroundHeight(MapId map, std::int32_t x, std::int32_t z, std::int32_t& y) const override;
};

const LegacyStageAdapter& ServerLegacyStageAdapter();
}
}
}
