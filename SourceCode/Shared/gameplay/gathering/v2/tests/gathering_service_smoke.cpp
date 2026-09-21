#include <cassert>

#include "systems/gameplay/gathering/v2/GatheringService.h"

int main()
{
    using namespace gameplay;
    gathering::GatheringService service;

    gathering::ResourceDefinition ore;
    ore.resourceId = 10;
    ore.key = "iron_ore";
    ore.kind = gathering::GatheringKind::Mining;
    ore.requiredProfession = professions::ProfessionType::Miner;
    ore.requiredLevel = 3;
    ore.minimumYield = 1;
    ore.maximumYield = 3;
    assert(service.RegisterResource(ore));

    gathering::GatheringNodeDefinition node;
    node.nodeId = 100;
    node.resourceId = 10;
    node.area = 7;
    node.x = 100;
    node.y = 0;
    node.z = 100;
    node.interactionRadius = 20;
    node.respawnMilliseconds = 5000;
    assert(service.RegisterNode(node));

    gathering::HarvestRequest request;
    request.playerName = "miner";
    request.profession = professions::ProfessionType::Miner;
    request.professionLevel = 3;
    request.nodeId = 100;
    request.area = 7;
    request.x = 105;
    request.y = 0;
    request.z = 100;
    request.nowMilliseconds = 1000;
    request.roll = 4;

    gathering::HarvestResult result = service.Harvest(request);
    assert(result.Succeeded());
    assert(result.amount == 2);
    assert(result.nextAvailableAt == 6000);

    result = service.Harvest(request);
    assert(result.status == gathering::HarvestStatus::Depleted);

    request.nowMilliseconds = 6000;
    request.x = 1000;
    result = service.Harvest(request);
    assert(result.status == gathering::HarvestStatus::TooFar);

    request.x = 100;
    request.professionLevel = 2;
    result = service.Harvest(request);
    assert(result.status == gathering::HarvestStatus::ProfessionLevelTooLow);
    return 0;
}
