#pragma once

#include <cstdint>
#include <string>

namespace gameplay
{
namespace events
{
struct FishCaughtEvent
{
    std::string playerName;
    std::uint32_t nodeId;
    std::uint32_t resourceId;
    std::string resourceKey;
    int amount;
    int area;

    FishCaughtEvent() : nodeId(0), resourceId(0), amount(0), area(-1) {}
};
}
}
