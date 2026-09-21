#pragma once

#include <cstdint>
#include <string>

#include "gameplay/events/v2/GameplayEventHub.h"
#include "gameplay/gathering/v2/GatheringTypes.h"
#include "gameplay/professions/v2/ProfessionService.h"
#include "systems/gameplay/gathering/v2/GatheringService.h"

namespace gameplay
{
namespace mining
{
struct MiningRequest
{
    std::string playerName;
    std::uint32_t nodeId;
    int area;
    int x;
    int y;
    int z;
    std::uint64_t nowMilliseconds;
    std::uint32_t roll;

    MiningRequest()
        : nodeId(0), area(-1), x(0), y(0), z(0), nowMilliseconds(0), roll(0) {}
};

class MiningService
{
public:
    MiningService(professions::ProfessionService& professions,
                  events::GameplayEventHub* eventHub = 0);

    bool RegisterResource(const gathering::ResourceDefinition& definition);
    bool RegisterNode(const gathering::GatheringNodeDefinition& definition);
    gathering::HarvestResult Harvest(const MiningRequest& request);

    gathering::GatheringService& Gathering() { return gathering_; }
    const gathering::GatheringService& Gathering() const { return gathering_; }

private:
    professions::ProfessionService& professions_;
    events::GameplayEventHub* eventHub_;
    gathering::GatheringService gathering_;
};
}
}
