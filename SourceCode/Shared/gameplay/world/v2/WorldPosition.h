#pragma once

#include <cstdint>

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{

// The legacy world stores positions in 24.8 fixed-point units. Keep this
// encoding explicit in GREEN code so consumers never need FLOATNS/fONE.
static constexpr int kWorldFixedFractionBits = 8;

inline std::int32_t WorldFixedToGrid(std::int32_t value)
{
    return value >> kWorldFixedFractionBits;
}

inline std::int32_t WorldGridToFixed(std::int32_t value)
{
    return value * (1 << kWorldFixedFractionBits);
}

// Coordinates are stored in the server's fixed-point world units. Conversion
// to tile/render units belongs in the integration/presentation boundary.
struct WorldPosition
{
    MapId map;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;

    WorldPosition() : map(), x(0), y(0), z(0) {}
    WorldPosition(MapId mapValue, std::int32_t xValue, std::int32_t yValue, std::int32_t zValue)
        : map(mapValue), x(xValue), y(yValue), z(zValue) {}
};
}
