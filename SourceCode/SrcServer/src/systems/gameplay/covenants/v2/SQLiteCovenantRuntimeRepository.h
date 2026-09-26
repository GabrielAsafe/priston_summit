#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include "gameplay/covenants/v2/CovenantRuntimeTypes.h"

namespace gameplay { namespace covenants {
class SQLiteCovenantRuntimeRepository
{
public:
    bool Ready();
    bool GetSwitchCooldownUntil(const std::string& playerKey, std::int64_t& value);
    bool SetSwitchCooldownUntil(const std::string& playerKey, std::int64_t value);
    bool GrantMastery(const std::string& playerKey, CovenantId covenantId, std::int64_t now);
    bool HasMastery(const std::string& playerKey, CovenantId covenantId, bool& value);
    bool ListMasteries(const std::string& playerKey, std::vector<CovenantId>& covenantIds);
    bool GrantCollectible(const std::string& playerKey, CovenantId covenantId, const std::string& key, std::int64_t now);
    bool ListCollectibles(const std::string& playerKey, std::vector<std::string>& keys);
    bool AddInventoryItem(const std::string& playerKey, CovenantId covenantId, const std::string& key, int amount, std::int64_t now);
    bool ConsumeInventoryItem(const std::string& playerKey, CovenantId covenantId, const std::string& key, int amount);
    bool ListInventoryItems(const std::string& playerKey, std::vector<CovenantInventoryItem>& items);
    bool GetArtisanMode(const std::string& playerKey, int& mode);
    bool SetArtisanMode(const std::string& playerKey, int mode);
    bool GetActivityReputation(CovenantId covenantId, const std::string& activityKey, int& reputationPerUnit);
    bool IncrementChallenges(const std::string& playerKey, CovenantId covenantId, const std::string& activityKey, int amount,
                             std::vector<CovenantChallengeState>& states);
    bool ListChallenges(const std::string& playerKey, CovenantId covenantId, std::vector<CovenantChallengeState>& states);
    bool MarkChallengeRewardClaimed(const std::string& playerKey, CovenantId covenantId, const std::string& challengeKey);
    bool AllChallengesCompleted(const std::string& playerKey, CovenantId covenantId, bool& completed);
    bool LoadPowerDefinition(CovenantId covenantId, const std::string& powerKey, CovenantPowerDefinition& definition);
    bool LoadPowerState(const std::string& playerKey, CovenantId covenantId, const std::string& powerKey, CovenantPowerState& state);
    bool SavePowerState(const std::string& playerKey, const CovenantPowerState& state);
    bool DeactivatePowers(const std::string& playerKey, CovenantId covenantId);
    bool ListActivePowerModifiers(const std::string& playerKey, CovenantId covenantId, std::int64_t now,
                                  std::uint32_t contextTargetSerial,
                                  std::vector<modifiers::GameplayModifier>& modifiers);
private:
    bool EnsureSchema();
};
} }
