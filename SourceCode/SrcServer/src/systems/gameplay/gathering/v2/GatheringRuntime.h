#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "gameplay/core/v2/GameplayIds.h"
#include "gameplay/gathering/v2/GatheringTypes.h"
#include "systems/gameplay/gathering/v2/GatheringService.h"
#include "systems/gameplay/gathering/v2/SQLiteGatheringRepository.h"


namespace gameplay
{
namespace gathering
{
bool InitializeServerGatheringRuntime(std::string* error = 0);
GatheringService& ServerGatheringService();
SQLiteGatheringRepository& ServerGatheringRepository();

HarvestResult HarvestGatheringNodeForPlayer(PlayerId player,
                                             std::uint32_t nodeId,
                                             std::uint64_t nowMilliseconds,
                                             std::uint32_t roll);

bool FindNearestGatheringNode(int area, int x, int y, int z,
                              GatheringKind kind,
                              bool exactKind,
                              GatheringNodeDefinition& outNode,
                              ResourceDefinition& outResource);

bool AddServerGatheringNode(const std::string& resourceKey,
                            int area, int x, int y, int z,
                            int interactionRadius,
                            std::uint64_t respawnMilliseconds,
                            GatheringNodeDefinition& outNode,
                            std::string* error = 0);
}
}
