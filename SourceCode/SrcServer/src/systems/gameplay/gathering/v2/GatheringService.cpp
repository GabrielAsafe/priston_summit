#include "systems/gameplay/gathering/v2/GatheringService.h"

#include <algorithm>
#include <cstdint>

namespace gameplay
{
namespace gathering
{
namespace
{
bool ValidResource(const ResourceDefinition& definition)
{
    return definition.resourceId != 0 && !definition.key.empty() &&
           definition.requiredLevel >= 0 && definition.minimumYield > 0 &&
           definition.maximumYield >= definition.minimumYield;
}

bool ValidNode(const GatheringNodeDefinition& definition)
{
    return definition.nodeId != 0 && definition.resourceId != 0 &&
           definition.area >= 0 && definition.interactionRadius >= 0;
}

std::uint64_t DistanceSquared(const HarvestRequest& request,
                              const GatheringNodeDefinition& node)
{
    const std::int64_t dx = static_cast<std::int64_t>(request.x) - node.x;
    const std::int64_t dy = static_cast<std::int64_t>(request.y) - node.y;
    const std::int64_t dz = static_cast<std::int64_t>(request.z) - node.z;
    return static_cast<std::uint64_t>(dx * dx + dy * dy + dz * dz);
}
}

bool GatheringService::RegisterResource(const ResourceDefinition& definition)
{
    if (!ValidResource(definition))
        return false;
    resources_[definition.resourceId] = definition;
    return true;
}

bool GatheringService::RegisterNode(const GatheringNodeDefinition& definition)
{
    if (!ValidNode(definition) || resources_.find(definition.resourceId) == resources_.end())
        return false;
    nodes_[definition.nodeId] = definition;
    return true;
}

bool GatheringService::RemoveNode(std::uint32_t nodeId)
{
    nextAvailableAt_.erase(nodeId);
    return nodes_.erase(nodeId) != 0;
}

HarvestResult GatheringService::Harvest(const HarvestRequest& request)
{
    HarvestResult result;
    result.nodeId = request.nodeId;
    if (request.playerName.empty())
    {
        result.status = HarvestStatus::InvalidPlayer;
        return result;
    }

    const std::unordered_map<std::uint32_t, GatheringNodeDefinition>::const_iterator nodeIt =
        nodes_.find(request.nodeId);
    if (nodeIt == nodes_.end())
    {
        result.status = HarvestStatus::UnknownNode;
        return result;
    }
    const GatheringNodeDefinition& node = nodeIt->second;
    result.area = node.area;
    result.resourceId = node.resourceId;

    const std::unordered_map<std::uint32_t, ResourceDefinition>::const_iterator resourceIt =
        resources_.find(node.resourceId);
    if (resourceIt == resources_.end())
    {
        result.status = HarvestStatus::UnknownResource;
        return result;
    }
    const ResourceDefinition& resource = resourceIt->second;
    result.resourceKey = resource.key;

    if (request.area != node.area)
    {
        result.status = HarvestStatus::WrongArea;
        return result;
    }

    const std::uint64_t radius = static_cast<std::uint64_t>(node.interactionRadius);
    if (DistanceSquared(request, node) > radius * radius)
    {
        result.status = HarvestStatus::TooFar;
        return result;
    }

    if (resource.requiredProfession != professions::ProfessionType::None &&
        request.profession != resource.requiredProfession)
    {
        result.status = HarvestStatus::WrongProfession;
        return result;
    }

    if (request.professionLevel < resource.requiredLevel)
    {
        result.status = HarvestStatus::ProfessionLevelTooLow;
        return result;
    }

    const std::uint64_t next = NextAvailableAt(node.nodeId);
    if (request.nowMilliseconds < next)
    {
        result.status = HarvestStatus::Depleted;
        result.nextAvailableAt = next;
        return result;
    }

    const int spread = resource.maximumYield - resource.minimumYield + 1;
    result.amount = resource.minimumYield + static_cast<int>(request.roll % static_cast<std::uint32_t>(spread));
    result.status = HarvestStatus::Success;
    result.nextAvailableAt = request.nowMilliseconds + node.respawnMilliseconds;
    nextAvailableAt_[node.nodeId] = result.nextAvailableAt;
    return result;
}

bool GatheringService::GetResource(std::uint32_t resourceId, ResourceDefinition& out) const
{
    const std::unordered_map<std::uint32_t, ResourceDefinition>::const_iterator it = resources_.find(resourceId);
    if (it == resources_.end())
        return false;
    out = it->second;
    return true;
}

bool GatheringService::GetNode(std::uint32_t nodeId, GatheringNodeDefinition& out) const
{
    const std::unordered_map<std::uint32_t, GatheringNodeDefinition>::const_iterator it = nodes_.find(nodeId);
    if (it == nodes_.end())
        return false;
    out = it->second;
    return true;
}

std::uint64_t GatheringService::NextAvailableAt(std::uint32_t nodeId) const
{
    const std::unordered_map<std::uint32_t, std::uint64_t>::const_iterator it = nextAvailableAt_.find(nodeId);
    return it == nextAvailableAt_.end() ? 0 : it->second;
}

void GatheringService::ResetNode(std::uint32_t nodeId)
{
    nextAvailableAt_.erase(nodeId);
}

void GatheringService::Clear()
{
    resources_.clear();
    nodes_.clear();
    nextAvailableAt_.clear();
}
}
}
