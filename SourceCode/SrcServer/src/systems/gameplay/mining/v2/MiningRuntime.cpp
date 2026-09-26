#include "systems/gameplay/mining/v2/MiningRuntime.h"

#include "systems/gameplay/events/v2/GameplayEventRuntime.h"
#include "systems/gameplay/professions/v2/ProfessionRuntime.h"
#include "systems/gameplay/progression/v2/ProgressionRuntime.h"

namespace gameplay
{
namespace mining
{
namespace
{
class ServerMiningCovenantBridge : public covenants::ICovenantGameplayBridge
{
public:
    double ApplyModifierValue(const std::string& playerKey,
                              const std::string& statKey,
                              const std::string& scopeKey,
                              double baseValue,
                              bool includeGlobalScope,
                              unsigned int contextTargetSerial) override
    {
        return progression::ApplyServerCovenantModifierValue(
            playerKey, statKey, scopeKey, baseValue,
            includeGlobalScope, contextTargetSerial);
    }

    bool GetModifierAggregate(const std::string& playerKey,
                              const std::string& statKey,
                              const std::string& scopeKey,
                              covenants::CovenantModifierAggregate& aggregate,
                              bool includeGlobalScope,
                              unsigned int contextTargetSerial) override
    {
        return progression::GetServerCovenantModifierAggregate(
            playerKey, statKey, scopeKey, aggregate,
            includeGlobalScope, contextTargetSerial);
    }

    bool AddInventoryItem(const std::string& playerKey,
                          int covenantId,
                          const std::string& itemKey,
                          int amount) override
    {
        return progression::AddServerCovenantInventoryItem(
            playerKey, covenantId, itemKey, amount);
    }

    bool RecordActivity(const std::string& playerKey,
                        const std::string& activityKey,
                        int amount) override
    {
        return progression::RecordServerCovenantActivity(
            playerKey, activityKey, amount);
    }
};

covenants::ICovenantGameplayBridge& CovenantBridge()
{
    static ServerMiningCovenantBridge bridge;
    return bridge;
}
}

MiningService& ServerMiningService()
{
    static MiningService service(professions::ServerProfessionService(),
                                 &events::ServerGameplayEventHub(),
                                 &CovenantBridge());
    return service;
}
}
}
