#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "gameplay/gathering/v2/IGatheringService.h"

namespace gameplay
{
namespace gathering
{
class GatheringService : public IGatheringService
{
public:
    bool RegisterResource(const ResourceDefinition& definition) override;
    bool RegisterNode(const GatheringNodeDefinition& definition) override;
    bool RemoveNode(std::uint32_t nodeId) override;

    HarvestResult Harvest(const HarvestRequest& request) override;

    bool GetResource(std::uint32_t resourceId, ResourceDefinition& out) const override;
    bool GetNode(std::uint32_t nodeId, GatheringNodeDefinition& out) const override;
    void ListResources(std::vector<ResourceDefinition>& out) const override;
    void ListNodes(std::vector<GatheringNodeDefinition>& out) const override;
    std::uint64_t NextAvailableAt(std::uint32_t nodeId) const override;
    void ResetNode(std::uint32_t nodeId) override;
    void Clear() override;

private:
    std::unordered_map<std::uint32_t, ResourceDefinition> resources_;
    std::unordered_map<std::uint32_t, GatheringNodeDefinition> nodes_;
    std::unordered_map<std::uint32_t, std::uint64_t> nextAvailableAt_;
};
}
}
