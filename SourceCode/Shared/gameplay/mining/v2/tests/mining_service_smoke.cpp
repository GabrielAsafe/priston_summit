#include <cassert>

#include "gameplay/events/v2/MiningNodeHarvestedEvent.h"
#include "systems/gameplay/mining/v2/MiningService.h"

int main()
{
    using namespace gameplay;
    professions::ProfessionService professions;
    professions.SetProgress("miner", professions::ProfessionType::Miner,
                            professions::ProfessionProgress(5, 0));

    events::GameplayEventHub hub;
    int published = 0;
    int publishedAmount = 0;
    hub.Subscribe<events::MiningNodeHarvestedEvent>(
        [&](const events::MiningNodeHarvestedEvent& event)
        {
            ++published;
            publishedAmount = event.amount;
            assert(event.playerName == "miner");
            assert(event.resourceKey == "copper_ore");
        });

    mining::MiningService service(professions, &hub);
    gathering::ResourceDefinition resource;
    resource.resourceId = 21;
    resource.key = "copper_ore";
    resource.kind = gathering::GatheringKind::Mining;
    resource.requiredProfession = professions::ProfessionType::Miner;
    resource.requiredLevel = 4;
    resource.minimumYield = 2;
    resource.maximumYield = 4;
    assert(service.RegisterResource(resource));

    gathering::GatheringNodeDefinition node;
    node.nodeId = 210;
    node.resourceId = 21;
    node.area = 2;
    node.x = 50;
    node.z = 75;
    node.interactionRadius = 16;
    node.respawnMilliseconds = 10000;
    assert(service.RegisterNode(node));

    mining::MiningRequest request;
    request.playerName = "miner";
    request.nodeId = 210;
    request.area = 2;
    request.x = 50;
    request.z = 75;
    request.nowMilliseconds = 500;
    request.roll = 5;

    const gathering::HarvestResult result = service.Harvest(request);
    assert(result.Succeeded());
    assert(result.amount == 4);
    assert(published == 1);
    assert(publishedAmount == 4);
    return 0;
}
