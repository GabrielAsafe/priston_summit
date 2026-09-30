#pragma once

#include "gameplay/world/v2/IWorldQuery.h"
#include "gameplay/world/v2/IStageQuery.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

namespace gameplay
{
namespace world
{
namespace legacy
{
class LegacyWorldAdapter final : public IWorldQuery
{
public:
    LegacyWorldAdapter(const player::legacy::LegacyOnlinePlayerRegistry& players, const IStageQuery& stages);

    bool TryGetPosition(EntityId entity, WorldPosition& out) const override;
    bool IsValidPosition(const WorldPosition& position) const override;
    bool TryGetGroundHeight(MapId map, std::int32_t x, std::int32_t z, std::int32_t& y) const override;
    bool IsSameArea(EntityId lhs, EntityId rhs) const override;

private:
    const player::legacy::LegacyOnlinePlayerRegistry& players_;
    const IStageQuery& stages_;
};

const LegacyWorldAdapter& ServerLegacyWorldAdapter();
}
}
}
