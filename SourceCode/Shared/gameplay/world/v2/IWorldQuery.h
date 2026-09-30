#pragma once

#include <cstdint>

#include "gameplay/core/v2/GameplayIds.h"
#include "gameplay/world/v2/WorldPosition.h"

namespace gameplay
{
namespace world
{
class IWorldQuery
{
public:
    virtual ~IWorldQuery() {}

    virtual bool TryGetPosition(EntityId entity, WorldPosition& out) const = 0;
    virtual bool IsValidPosition(const WorldPosition& position) const = 0;
    virtual bool TryGetGroundHeight(MapId map,
                                    std::int32_t x,
                                    std::int32_t z,
                                    std::int32_t& y) const = 0;
    virtual bool IsSameArea(EntityId lhs, EntityId rhs) const = 0;
};
}
}
