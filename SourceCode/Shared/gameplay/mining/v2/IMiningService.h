#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "gameplay/gathering/v2/GatheringTypes.h"

namespace gameplay
{
namespace mining
{
struct MiningRequest
{
    std::string playerName;
    std::uint32_t nodeId;
    int area;
    int x;
    int y;
    int z;
    std::uint64_t nowMilliseconds;
    std::uint32_t roll;

    MiningRequest()
        : nodeId(0), area(-1), x(0), y(0), z(0), nowMilliseconds(0), roll(0) {}
};

class IMiningService
{
public:
    virtual ~IMiningService() {}

    virtual bool RegisterResource(const gathering::ResourceDefinition& definition) = 0;
    virtual bool RegisterNode(const gathering::GatheringNodeDefinition& definition) = 0;
    virtual gathering::HarvestResult Harvest(const MiningRequest& request) = 0;
    virtual void ListResources(std::vector<gathering::ResourceDefinition>& out) const = 0;
    virtual void ListNodes(std::vector<gathering::GatheringNodeDefinition>& out) const = 0;
};
}
}
