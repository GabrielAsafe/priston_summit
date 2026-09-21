#include "systems/gameplay/mining/v2/MiningInteractionRuntime.h"

#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/mining/v2/MiningInteractionService.h"
#include "systems/gameplay/mining/v2/MiningRuntime.h"

namespace gameplay
{
namespace mining
{
gathering::HarvestResult HarvestMiningNodeForPlayer(rsPLAYINFO* player,
                                                     std::uint32_t nodeId,
                                                     std::uint64_t nowMilliseconds,
                                                     std::uint32_t roll)
{
    if (!player)
    {
        gathering::HarvestResult invalid;
        invalid.status = gathering::HarvestStatus::InvalidPlayer;
        invalid.nodeId = nodeId;
        return invalid;
    }

    MiningActorState actor;
    const char* name = player->getName();
    actor.playerName = name ? name : "";
    actor.area = player->Position.Area;
    actor.x = player->Position.x >> FLOATNS;
    actor.y = player->Position.y >> FLOATNS;
    actor.z = player->Position.z >> FLOATNS;

    static MiningInteractionService interaction(ServerMiningService());
    return interaction.Harvest(actor, nodeId, nowMilliseconds, roll);
}
}
}
