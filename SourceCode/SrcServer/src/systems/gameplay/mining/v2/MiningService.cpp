#include "systems/gameplay/mining/v2/MiningService.h"

#include "gameplay/events/v2/MiningNodeHarvestedEvent.h"

namespace gameplay
{
namespace mining
{
MiningService::MiningService(professions::ProfessionService& professions,
                             events::GameplayEventHub* eventHub)
    : professions_(professions), eventHub_(eventHub)
{
}

bool MiningService::RegisterResource(const gathering::ResourceDefinition& definition)
{
    if (definition.kind != gathering::GatheringKind::Mining)
        return false;
    return gathering_.RegisterResource(definition);
}

bool MiningService::RegisterNode(const gathering::GatheringNodeDefinition& definition)
{
    gathering::ResourceDefinition resource;
    if (!gathering_.GetResource(definition.resourceId, resource) ||
        resource.kind != gathering::GatheringKind::Mining)
        return false;
    return gathering_.RegisterNode(definition);
}

gathering::HarvestResult MiningService::Harvest(const MiningRequest& request)
{
    gathering::HarvestRequest gatheringRequest;
    gatheringRequest.playerName = request.playerName;
    gatheringRequest.profession = professions::ProfessionType::Miner;
    gatheringRequest.professionLevel = professions_.GetLevel(request.playerName, professions::ProfessionType::Miner);
    gatheringRequest.nodeId = request.nodeId;
    gatheringRequest.area = request.area;
    gatheringRequest.x = request.x;
    gatheringRequest.y = request.y;
    gatheringRequest.z = request.z;
    gatheringRequest.nowMilliseconds = request.nowMilliseconds;
    gatheringRequest.roll = request.roll;

    gathering::HarvestResult result = gathering_.Harvest(gatheringRequest);
    if (result.Succeeded() && eventHub_)
    {
        events::MiningNodeHarvestedEvent event;
        event.playerName = request.playerName;
        event.nodeId = result.nodeId;
        event.resourceId = result.resourceId;
        event.resourceKey = result.resourceKey;
        event.amount = result.amount;
        event.area = result.area;
        eventHub_->Publish(event);
    }
    return result;
}
}
}
