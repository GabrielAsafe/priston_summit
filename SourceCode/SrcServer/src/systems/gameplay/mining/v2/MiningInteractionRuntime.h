#pragma once

#include <cstdint>

#include "gameplay/core/v2/GameplayIds.h"
#include "gameplay/gathering/v2/GatheringTypes.h"

namespace gameplay
{
namespace mining
{
// Server boundary for mining interaction. Legacy player resolution stays behind
// the runtime boundary; callers pass only stable gameplay identity.
gathering::HarvestResult HarvestMiningNodeForPlayer(PlayerId player,
                                                     std::uint32_t nodeId,
                                                     std::uint64_t nowMilliseconds,
                                                     std::uint32_t roll);
}
}
