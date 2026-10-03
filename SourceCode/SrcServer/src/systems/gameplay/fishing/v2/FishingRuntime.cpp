#include "systems/gameplay/fishing/v2/FishingRuntime.h"

#include "systems/gameplay/events/v2/GameplayEventRuntime.h"
#include "systems/gameplay/gathering/v2/GatheringRuntime.h"
#include "systems/gameplay/gathering/v2/legacy/LegacyHarvestPlayerQueryAdapter.h"
#include "systems/gameplay/professions/v2/ProfessionRuntime.h"
#include "systems/gameplay/progression/v2/ProgressionRuntime.h"

namespace gameplay
{
namespace fishing
{
namespace
{
class ServerFishingCovenantBridge : public covenants::ICovenantGameplayBridge
{
public:
    double ApplyModifierValue(const std::string& playerKey, const std::string& statKey,
                              const std::string& scopeKey, double baseValue,
                              bool includeGlobalScope, unsigned int contextTargetSerial) override
    {
        return progression::ApplyServerCovenantModifierValue(playerKey, statKey, scopeKey,
            baseValue, includeGlobalScope, contextTargetSerial);
    }
    bool GetModifierAggregate(const std::string& playerKey, const std::string& statKey,
                              const std::string& scopeKey, covenants::CovenantModifierAggregate& aggregate,
                              bool includeGlobalScope, unsigned int contextTargetSerial) override
    {
        return progression::GetServerCovenantModifierAggregate(playerKey, statKey, scopeKey,
            aggregate, includeGlobalScope, contextTargetSerial);
    }
    bool AddInventoryItem(const std::string& playerKey, int covenantId,
                          const std::string& itemKey, int amount) override
    {
        return progression::AddServerCovenantInventoryItem(playerKey, covenantId, itemKey, amount);
    }
    bool RecordActivity(const std::string& playerKey, const std::string& activityKey, int amount) override
    {
        return progression::RecordServerCovenantActivity(playerKey, activityKey, amount);
    }
};

covenants::ICovenantGameplayBridge& CovenantBridge()
{
    static ServerFishingCovenantBridge bridge;
    return bridge;
}
}

FishingService& ServerFishingService()
{
    static FishingService service(professions::ServerProfessionService(),
                                  &events::ServerGameplayEventHub(), &CovenantBridge());
    return service;
}

gathering::HarvestResult FishNodeForPlayer(PlayerId playerId,
                                            std::uint32_t nodeId,
                                            std::uint64_t nowMilliseconds,
                                            std::uint32_t roll)
{
    gathering::HarvestResult invalid;
    invalid.nodeId = nodeId;
    gathering::HarvestPlayerSnapshot player;
    if (!gathering::legacy::ServerLegacyHarvestPlayerQueryAdapter().TryGet(playerId, player))
    {
        invalid.status = gathering::HarvestStatus::InvalidPlayer;
        return invalid;
    }

    FishingRequest request;
    request.playerName = player.playerName;
    request.nodeId = nodeId;
    request.area = player.area;
    request.x = player.x;
    request.y = player.y;
    request.z = player.z;
    request.nowMilliseconds = nowMilliseconds;
    request.roll = roll;

    gathering::HarvestResult result = ServerFishingService().Fish(request);
    if (result.Succeeded())
        (void)gathering::ServerGatheringRepository().AddInventory(request.playerName, result.resourceKey, result.amount);
    return result;
}
}
}
