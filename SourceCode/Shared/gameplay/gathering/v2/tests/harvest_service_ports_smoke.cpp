#include "gameplay/gathering/v2/IGatheringService.h"
#include "gameplay/mining/v2/IMiningService.h"
#include "systems/gameplay/gathering/v2/GatheringService.h"
#include "systems/gameplay/mining/v2/MiningInteractionService.h"

#include <cassert>
#include <iostream>

namespace
{
class FakeMiningService final : public gameplay::mining::IMiningService
{
public:
    gameplay::mining::MiningRequest last;
    bool RegisterResource(const gameplay::gathering::ResourceDefinition&) override { return true; }
    bool RegisterNode(const gameplay::gathering::GatheringNodeDefinition&) override { return true; }
    gameplay::gathering::HarvestResult Harvest(const gameplay::mining::MiningRequest& request) override
    {
        last = request;
        gameplay::gathering::HarvestResult result;
        result.status = gameplay::gathering::HarvestStatus::Success;
        result.nodeId = request.nodeId;
        return result;
    }
    void ListResources(std::vector<gameplay::gathering::ResourceDefinition>& out) const override { out.clear(); }
    void ListNodes(std::vector<gameplay::gathering::GatheringNodeDefinition>& out) const override { out.clear(); }
};
}

int main()
{
    gameplay::gathering::GatheringService concrete;
    gameplay::gathering::IGatheringService& gathering = concrete;
    gameplay::gathering::ResourceDefinition resource;
    resource.resourceId = 1;
    resource.key = "ore";
    resource.kind = gameplay::gathering::GatheringKind::Mining;
    assert(gathering.RegisterResource(resource));

    FakeMiningService fake;
    gameplay::mining::MiningInteractionService interaction(fake);
    gameplay::mining::MiningActorState actor;
    actor.playerName = "miner";
    actor.area = 7;
    actor.x = 10;
    actor.y = 20;
    actor.z = 30;
    const gameplay::gathering::HarvestResult result = interaction.Harvest(actor, 99, 1234, 5);
    assert(result.Succeeded());
    assert(fake.last.playerName == "miner");
    assert(fake.last.nodeId == 99);
    assert(fake.last.area == 7);
    assert(fake.last.nowMilliseconds == 1234);
    assert(fake.last.roll == 5);

    std::cout << "harvest service ports smoke: PASS\n";
    return 0;
}
