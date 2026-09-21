#include <cassert>

#include "systems/gameplay/mining/v2/MiningInteractionService.h"

int main()
{
    using namespace gameplay;

    professions::ProfessionService professions;
    professions.SetProgress("authoritative-miner", professions::ProfessionType::Miner,
                            professions::ProfessionProgress(3, 0));

    mining::MiningService mining(professions);

    gathering::ResourceDefinition ore;
    ore.resourceId = 30;
    ore.key = "tin_ore";
    ore.kind = gathering::GatheringKind::Mining;
    ore.requiredProfession = professions::ProfessionType::Miner;
    ore.requiredLevel = 3;
    ore.minimumYield = 1;
    ore.maximumYield = 1;
    assert(mining.RegisterResource(ore));

    gathering::GatheringNodeDefinition node;
    node.nodeId = 300;
    node.resourceId = 30;
    node.area = 11;
    node.x = 120;
    node.y = 5;
    node.z = 220;
    node.interactionRadius = 12;
    node.respawnMilliseconds = 2500;
    assert(mining.RegisterNode(node));

    mining::MiningInteractionService interaction(mining);
    mining::MiningActorState actor;
    actor.playerName = "authoritative-miner";
    actor.area = 11;
    actor.x = 122;
    actor.y = 5;
    actor.z = 220;

    gathering::HarvestResult result = interaction.Harvest(actor, 300, 1000, 0);
    assert(result.Succeeded());
    assert(result.resourceKey == "tin_ore");

    actor.x = 1000;
    result = interaction.Harvest(actor, 300, 4000, 0);
    assert(result.status == gathering::HarvestStatus::TooFar);
    return 0;
}
