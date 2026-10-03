#pragma once

#include "gameplay/covenants/v2/ICovenantGameplayBridge.h"
#include "gameplay/events/v2/GameplayEventHub.h"
#include "gameplay/fishing/v2/IFishingService.h"
#include "gameplay/professions/v2/IProfessionService.h"
#include "systems/gameplay/gathering/v2/GatheringService.h"

namespace gameplay
{
namespace fishing
{
class FishingService : public IFishingService
{
public:
    FishingService(professions::IProfessionService& professions,
                   events::GameplayEventHub* eventHub = 0,
                   covenants::ICovenantGameplayBridge* covenantBridge = 0);

    bool RegisterResource(const gathering::ResourceDefinition& definition) override;
    bool RegisterNode(const gathering::GatheringNodeDefinition& definition) override;
    gathering::HarvestResult Fish(const FishingRequest& request) override;
    void ListResources(std::vector<gathering::ResourceDefinition>& out) const override;
    void ListNodes(std::vector<gathering::GatheringNodeDefinition>& out) const override;

    gathering::GatheringService& Gathering() { return gathering_; }
    const gathering::GatheringService& Gathering() const { return gathering_; }

private:
    professions::IProfessionService& professions_;
    events::GameplayEventHub* eventHub_;
    covenants::ICovenantGameplayBridge* covenantBridge_;
    gathering::GatheringService gathering_;
};
}
}
