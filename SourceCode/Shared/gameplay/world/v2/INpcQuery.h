#pragma once

#include <cstdint>

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace world
{
struct NpcSnapshot
{
    EntityId entity;
    std::uint32_t eventCode;

    NpcSnapshot() : entity(), eventCode(0u) {}
    NpcSnapshot(EntityId entityValue, std::uint32_t eventCodeValue)
        : entity(entityValue), eventCode(eventCodeValue) {}
};

class INpcQuery
{
public:
    virtual ~INpcQuery() {}
    virtual bool TryGetNpc(EntityId entity, NpcSnapshot& out) const = 0;
};
}
}
