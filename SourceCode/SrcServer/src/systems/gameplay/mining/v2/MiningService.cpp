#include "systems/gameplay/mining/v2/MiningService.h"

#include "gameplay/events/v2/MiningNodeHarvestedEvent.h"
#include "gameplay/covenants/v2/CovenantCatalog.h"

namespace gameplay
{
namespace mining
{
MiningService::MiningService(professions::ProfessionService& professions,
                             events::GameplayEventHub* eventHub,
                             covenants::ICovenantGameplayBridge* covenantBridge)
    : professions_(professions), eventHub_(eventHub), covenantBridge_(covenantBridge)
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

void MiningService::ListResources(std::vector<gathering::ResourceDefinition>& out) const
{
    gathering_.ListResources(out);
}

void MiningService::ListNodes(std::vector<gathering::GatheringNodeDefinition>& out) const
{
    gathering_.ListNodes(out);
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
    if (result.Succeeded())
    {
        const int baseAmount = result.amount;
        const double adjustedYield = covenantBridge_
            ? covenantBridge_->ApplyModifierValue(
                request.playerName, "gathering.yield", "gathering",
                static_cast<double>(result.amount))
            : static_cast<double>(result.amount);
        result.amount = adjustedYield >= 1.0 ? static_cast<int>(adjustedYield) : 1;

        covenants::CovenantModifierAggregate extraYield;
        if (covenantBridge_ && covenantBridge_->GetModifierAggregate(
                request.playerName, "gathering.extra_yield_chance", "gathering",
                extraYield) && extraYield.matched)
        {
            double chance = extraYield.flat + extraYield.additivePercent;
            if (extraYield.multiplicativeFactor != 1.0)
                chance *= extraYield.multiplicativeFactor;
            if (chance > 1.0) chance = 1.0;
            const std::uint32_t covenantRoll = request.roll * 1103515245u + 12345u;
            if (chance > 0.0 && (covenantRoll % 10000u) <
                    static_cast<std::uint32_t>(chance * 10000.0))
                result.amount += baseAmount;
        }

        // Special/refined Gatherer materials are Covenant-inventory stacks.
        // They can later be bridged to legacy item IDs/recipes without changing
        // the progression contract or the harvest hot path.
        covenants::CovenantModifierAggregate specialMaterial;
        if (covenantBridge_ && covenantBridge_->GetModifierAggregate(
                request.playerName, "gathering.special_material_chance", "gathering",
                specialMaterial) && specialMaterial.matched)
        {
            double chance = specialMaterial.flat + specialMaterial.additivePercent;
            if (specialMaterial.multiplicativeFactor != 1.0) chance *= specialMaterial.multiplicativeFactor;
            if (chance > 1.0) chance = 1.0;
            const std::uint32_t materialRoll = request.roll * 1664525u + 1013904223u;
            if (chance > 0.0 && (materialRoll % 10000u) < static_cast<std::uint32_t>(chance * 10000.0))
                (void)covenantBridge_->AddInventoryItem(
                    request.playerName, covenants::catalog::kGatherers,
                    std::string("special_material:") + result.resourceKey, 1);
        }

        covenants::CovenantModifierAggregate refinedMaterial;
        if (covenantBridge_ && covenantBridge_->GetModifierAggregate(
                request.playerName, "gathering.refined_material_chance", "gathering",
                refinedMaterial) && refinedMaterial.matched)
        {
            double chance = refinedMaterial.flat + refinedMaterial.additivePercent;
            if (refinedMaterial.multiplicativeFactor != 1.0) chance *= refinedMaterial.multiplicativeFactor;
            if (chance > 1.0) chance = 1.0;
            const std::uint32_t refinedRoll = request.roll * 22695477u + 1u;
            if (chance > 0.0 && (refinedRoll % 10000u) < static_cast<std::uint32_t>(chance * 10000.0))
                (void)covenantBridge_->AddInventoryItem(
                    request.playerName, covenants::catalog::kGatherers,
                    std::string("refined_material:") + result.resourceKey, 1);
        }

        if (covenantBridge_)
            (void)covenantBridge_->RecordActivity(
                request.playerName, "gathering.harvest", 1);
    }

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
