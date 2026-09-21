#pragma once

#include <cstdint>
#include <unordered_map>

#include "gameplay/gathering/v2/GatheringTypes.h"

namespace gameplay
{
namespace gathering
{
class GatheringService
{
public:
    bool RegisterResource(const ResourceDefinition& definition);
    bool RegisterNode(const GatheringNodeDefinition& definition);
    bool RemoveNode(std::uint32_t nodeId);

    HarvestResult Harvest(const HarvestRequest& request);

    bool GetResource(std::uint32_t resourceId, ResourceDefinition& out) const;
    bool GetNode(std::uint32_t nodeId, GatheringNodeDefinition& out) const;
    std::uint64_t NextAvailableAt(std::uint32_t nodeId) const;
    void ResetNode(std::uint32_t nodeId);
    void Clear();

private:
    std::unordered_map<std::uint32_t, ResourceDefinition> resources_;
    std::unordered_map<std::uint32_t, GatheringNodeDefinition> nodes_;
    std::unordered_map<std::uint32_t, std::uint64_t> nextAvailableAt_;
};
}
}
