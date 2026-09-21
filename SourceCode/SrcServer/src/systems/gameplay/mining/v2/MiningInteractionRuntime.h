#pragma once

#include <cstdint>

#include "gameplay/gathering/v2/GatheringTypes.h"

class rsPLAYINFO;

namespace gameplay
{
namespace mining
{
// Server boundary for mining interaction. Player identity and position are
// always derived from rsPLAYINFO; clients only need to identify the node.
gathering::HarvestResult HarvestMiningNodeForPlayer(rsPLAYINFO* player,
                                                     std::uint32_t nodeId,
                                                     std::uint64_t nowMilliseconds,
                                                     std::uint32_t roll);
}
}
