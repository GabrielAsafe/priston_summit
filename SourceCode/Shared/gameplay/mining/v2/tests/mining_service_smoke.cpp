#include <cassert>
#include <map>
#include <string>

#include "gameplay/professions/v2/ProfessionService.h"
#include "gameplay/covenants/v2/ICovenantGameplayBridge.h"

#include "gameplay/events/v2/MiningNodeHarvestedEvent.h"
#include "systems/gameplay/mining/v2/MiningService.h"


namespace
{
class FakeCovenantBridge : public gameplay::covenants::ICovenantGameplayBridge
{
public:
    std::map<std::string, int> inventory;
    int activities = 0;

    double ApplyModifierValue(const std::string&,
                              const std::string& statKey,
                              const std::string&,
                              double baseValue,
                              bool, unsigned int) override
    {
        if (statKey == "gathering.yield")
            return baseValue * 1.5;
        return baseValue;
    }

    bool GetModifierAggregate(const std::string&,
                              const std::string& statKey,
                              const std::string&,
                              gameplay::covenants::CovenantModifierAggregate& aggregate,
                              bool, unsigned int) override
    {
        aggregate = gameplay::covenants::CovenantModifierAggregate();
        if (statKey == "gathering.extra_yield_chance" ||
            statKey == "gathering.special_material_chance" ||
            statKey == "gathering.refined_material_chance")
        {
            aggregate.matched = true;
            aggregate.additivePercent = 1.0;
            return true;
        }
        return false;
    }

    bool AddInventoryItem(const std::string&, int,
                          const std::string& itemKey, int amount) override
    {
        inventory[itemKey] += amount;
        return true;
    }

    bool RecordActivity(const std::string&, const std::string& activityKey,
                        int amount) override
    {
        if (activityKey == "gathering.harvest")
            activities += amount;
        return true;
    }
};
}

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

    // Domain service remains independently testable while the server can inject
    // Covenant effects through a narrow bridge. Force every chance to 100% so
    // this smoke deterministically exercises yield, Covenant inventory and the
    // repeatable activity hook.
    FakeCovenantBridge covenantBridge;
    mining::MiningService covenantService(professions, 0, &covenantBridge);
    assert(covenantService.RegisterResource(resource));

    gathering::GatheringNodeDefinition covenantNode = node;
    covenantNode.nodeId = 211;
    assert(covenantService.RegisterNode(covenantNode));

    mining::MiningRequest covenantRequest = request;
    covenantRequest.nodeId = covenantNode.nodeId;
    covenantRequest.nowMilliseconds = 25000;
    const gathering::HarvestResult covenantResult = covenantService.Harvest(covenantRequest);
    assert(covenantResult.Succeeded());
    assert(covenantResult.amount == 10); // 4 * 1.5 + one extra base yield (4)
    assert(covenantBridge.inventory["special_material:copper_ore"] == 1);
    assert(covenantBridge.inventory["refined_material:copper_ore"] == 1);
    assert(covenantBridge.activities == 1);
    return 0;
}
