#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "gameplay/events/v2/GameplayEventHub.h"
#include "gameplay/covenants/v2/ICovenantGameplayBridge.h"
#include "gameplay/gathering/v2/GatheringTypes.h"
#include "gameplay/mining/v2/IMiningService.h"
#include "gameplay/professions/v2/IProfessionService.h"
#include "systems/gameplay/gathering/v2/GatheringService.h"

namespace gameplay
{
namespace mining
{
class MiningService : public IMiningService
{
public:
    MiningService(professions::IProfessionService& professions,
                  events::GameplayEventHub* eventHub = 0,
                  covenants::ICovenantGameplayBridge* covenantBridge = 0);

    bool RegisterResource(const gathering::ResourceDefinition& definition) override;
    bool RegisterNode(const gathering::GatheringNodeDefinition& definition) override;
    gathering::HarvestResult Harvest(const MiningRequest& request) override;
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
