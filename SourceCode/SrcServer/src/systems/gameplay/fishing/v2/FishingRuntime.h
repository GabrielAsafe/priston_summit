#pragma once

#include <cstdint>

#include "gameplay/core/v2/GameplayIds.h"
#include "gameplay/gathering/v2/GatheringTypes.h"
#include "systems/gameplay/fishing/v2/FishingService.h"

namespace gameplay
{
namespace fishing
{
FishingService& ServerFishingService();
gathering::HarvestResult FishNodeForPlayer(PlayerId player,
                                            std::uint32_t nodeId,
                                            std::uint64_t nowMilliseconds,
                                            std::uint32_t roll);
}
}
