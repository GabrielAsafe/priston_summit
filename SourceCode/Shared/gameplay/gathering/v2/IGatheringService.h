#pragma once

#include <cstdint>
#include <vector>

#include "gameplay/gathering/v2/GatheringTypes.h"

namespace gameplay
{
namespace gathering
{
class IGatheringService
{
public:
    virtual ~IGatheringService() {}

    virtual bool RegisterResource(const ResourceDefinition& definition) = 0;
    virtual bool RegisterNode(const GatheringNodeDefinition& definition) = 0;
    virtual bool RemoveNode(std::uint32_t nodeId) = 0;
    virtual HarvestResult Harvest(const HarvestRequest& request) = 0;

    virtual bool GetResource(std::uint32_t resourceId, ResourceDefinition& out) const = 0;
    virtual bool GetNode(std::uint32_t nodeId, GatheringNodeDefinition& out) const = 0;
    virtual void ListResources(std::vector<ResourceDefinition>& out) const = 0;
    virtual void ListNodes(std::vector<GatheringNodeDefinition>& out) const = 0;
    virtual std::uint64_t NextAvailableAt(std::uint32_t nodeId) const = 0;
    virtual void ResetNode(std::uint32_t nodeId) = 0;
    virtual void Clear() = 0;
};
}
}
