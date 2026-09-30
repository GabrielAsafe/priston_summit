#pragma once

#include <cstdint>
#include <string>

#include "gameplay/mining/v2/IMiningService.h"

namespace gameplay
{
namespace mining
{
struct MiningActorState
{
    std::string playerName;
    int area;
    int x;
    int y;
    int z;

    MiningActorState() : area(-1), x(0), y(0), z(0) {}
};

class MiningInteractionService
{
public:
    explicit MiningInteractionService(IMiningService& mining) : mining_(mining) {}

    gathering::HarvestResult Harvest(const MiningActorState& actor,
                                     std::uint32_t nodeId,
                                     std::uint64_t nowMilliseconds,
                                     std::uint32_t roll)
    {
        MiningRequest request;
        request.playerName = actor.playerName;
        request.nodeId = nodeId;
        request.area = actor.area;
        request.x = actor.x;
        request.y = actor.y;
        request.z = actor.z;
        request.nowMilliseconds = nowMilliseconds;
        request.roll = roll;
        return mining_.Harvest(request);
    }

private:
    IMiningService& mining_;
};
}
}
