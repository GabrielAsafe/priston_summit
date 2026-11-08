#include "systems/gameplay/fishing/v2/FishingService.h"

#include <algorithm>

#include "gameplay/covenants/v2/CovenantCatalog.h"
#include "gameplay/events/v2/FishCaughtEvent.h"

namespace gameplay
{
namespace fishing
{
FishingService::FishingService(professions::IProfessionService& professions,
                               events::GameplayEventHub* eventHub,
                               covenants::ICovenantGameplayBridge* covenantBridge)
    : professions_(professions), eventHub_(eventHub), covenantBridge_(covenantBridge)
{
}

bool FishingService::RegisterResource(const gathering::ResourceDefinition& definition)
{
    if (definition.kind != gathering::GatheringKind::Fishing ||
        definition.requiredProfession != professions::ProfessionType::Fisher)
        return false;
    return gathering_.RegisterResource(definition);
}

bool FishingService::RegisterNode(const gathering::GatheringNodeDefinition& definition)
{
    gathering::ResourceDefinition resource;
    if (!gathering_.GetResource(definition.resourceId, resource) ||
        resource.kind != gathering::GatheringKind::Fishing)
        return false;
    return gathering_.RegisterNode(definition);
}

void FishingService::ListResources(std::vector<gathering::ResourceDefinition>& out) const
{
    gathering_.ListResources(out);
}

void FishingService::ListNodes(std::vector<gathering::GatheringNodeDefinition>& out) const
{
    gathering_.ListNodes(out);
}

gathering::HarvestResult FishingService::Fish(const FishingRequest& request)
{
    gathering::HarvestRequest harvest;
    harvest.playerName = request.playerName;
    harvest.profession = professions::ProfessionType::Fisher;
    harvest.professionLevel = professions_.GetLevel(request.playerName, professions::ProfessionType::Fisher);
    harvest.nodeId = request.nodeId;
    harvest.area = request.area;
    harvest.x = request.x;
    harvest.y = request.y;
    harvest.z = request.z;
    harvest.nowMilliseconds = request.nowMilliseconds;
    harvest.roll = request.roll;

    gathering::HarvestResult result = gathering_.Harvest(harvest);
    if (!result.Succeeded())
        return result;

    if (covenantBridge_)
    {
        const double adjusted = covenantBridge_->ApplyModifierValue(
            request.playerName, "gathering.yield", "fishing", static_cast<double>(result.amount));
        result.amount = std::max(1, static_cast<int>(adjusted));
        // Runtime records this activity only after durable material persistence.
    }

    // Persistence and Fisher XP are committed by FishingRuntime after
    // AddInventory succeeds. Avoid awarding XP on uncommitted catches.
    return result;
}
}
}
