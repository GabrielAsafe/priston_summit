#include "systems/gameplay/mining/v2/MiningInteractionRuntime.h"

#include "systems/gameplay/mining/v2/MiningInteractionService.h"
#include "systems/gameplay/mining/v2/MiningRuntime.h"
#include "systems/gameplay/gathering/v2/GatheringRuntime.h"
#include "systems/gameplay/gathering/v2/legacy/LegacyHarvestPlayerQueryAdapter.h"

namespace gameplay
{
namespace mining
{
gathering::HarvestResult HarvestMiningNodeForPlayer(PlayerId playerId,
                                                     std::uint32_t nodeId,
                                                     std::uint64_t nowMilliseconds,
                                                     std::uint32_t roll)
{
    gathering::HarvestPlayerSnapshot player;
    if (!gathering::legacy::ServerLegacyHarvestPlayerQueryAdapter().TryGet(playerId, player))
    {
        gathering::HarvestResult invalid;
        invalid.status = gathering::HarvestStatus::InvalidPlayer;
        invalid.nodeId = nodeId;
        return invalid;
    }

    MiningActorState actor;
    actor.playerName = player.playerName;
    actor.area = player.area;
    actor.x = player.x;
    actor.y = player.y;
    actor.z = player.z;

    static MiningInteractionService interaction(ServerMiningService());
    gathering::HarvestResult result = interaction.Harvest(actor, nodeId, nowMilliseconds, roll);
    if (result.Succeeded())
        (void)gathering::ServerGatheringRepository().AddInventory(actor.playerName, result.resourceKey, result.amount);
    return result;
}
}
}
