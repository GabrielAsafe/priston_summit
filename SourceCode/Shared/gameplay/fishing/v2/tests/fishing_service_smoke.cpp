#include <cassert>

#include "gameplay/events/v2/FishCaughtEvent.h"
#include "gameplay/professions/v2/ProfessionService.h"
#include "systems/gameplay/fishing/v2/FishingService.h"

int main()
{
    using namespace gameplay;
    professions::ProfessionService professions;
    professions.SetProgress("angler", professions::ProfessionType::Fisher,
                            professions::ProfessionProgress(3, 10));

    events::GameplayEventHub hub;
    int eventsSeen = 0;
    hub.Subscribe<events::FishCaughtEvent>([&](const events::FishCaughtEvent& event) {
        ++eventsSeen;
        assert(event.playerName == "angler");
        assert(event.resourceKey == "river_trout");
        assert(event.amount == 2);
    });

    fishing::FishingService service(professions, &hub);
    gathering::ResourceDefinition fish;
    fish.resourceId = 4001;
    fish.key = "river_trout";
    fish.kind = gathering::GatheringKind::Fishing;
    fish.requiredProfession = professions::ProfessionType::Fisher;
    fish.requiredLevel = 1;
    fish.minimumYield = 1;
    fish.maximumYield = 2;
    assert(service.RegisterResource(fish));

    gathering::ResourceDefinition invalid = fish;
    invalid.resourceId = 4002;
    invalid.kind = gathering::GatheringKind::Mining;
    assert(!service.RegisterResource(invalid));

    gathering::GatheringNodeDefinition node;
    node.nodeId = 41;
    node.resourceId = fish.resourceId;
    node.area = 5;
    node.x = 100;
    node.z = 100;
    node.interactionRadius = 20;
    node.respawnMilliseconds = 5000;
    assert(service.RegisterNode(node));

    fishing::FishingRequest request;
    request.playerName = "angler";
    request.nodeId = node.nodeId;
    request.area = 5;
    request.x = 105;
    request.z = 100;
    request.nowMilliseconds = 1000;
    request.roll = 1;

    const gathering::HarvestResult result = service.Fish(request);
    assert(result.Succeeded());
    assert(result.amount == 2);
    assert(eventsSeen == 1);
    assert(professions.GetExperience("angler", professions::ProfessionType::Fisher) == 12);

    const gathering::HarvestResult depleted = service.Fish(request);
    assert(depleted.status == gathering::HarvestStatus::Depleted);
    return 0;
}
