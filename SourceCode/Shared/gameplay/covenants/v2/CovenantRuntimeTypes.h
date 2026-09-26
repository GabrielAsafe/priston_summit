#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include "gameplay/covenants/v2/CovenantTypes.h"
#include "gameplay/modifiers/v2/GameplayModifier.h"

namespace gameplay { namespace covenants {

struct CovenantChallengeState
{
    CovenantId covenantId;
    std::string key;
    std::string activityKey;
    int targetCount;
    int progress;
    int reputationReward;
    bool completed;
    bool rewardClaimed;
    CovenantChallengeState() : covenantId(0), targetCount(0), progress(0), reputationReward(0), completed(false), rewardClaimed(false) {}
};

enum class CovenantPowerTargetMode { Self = 0, HuntTarget = 1 };

struct CovenantPowerDefinition
{
    CovenantId covenantId;
    std::string key;
    CovenantRankId requiredRankId;
    int durationSeconds;
    int cooldownSeconds;
    CovenantPowerTargetMode targetMode;
    std::vector<modifiers::GameplayModifier> modifiers;
    CovenantPowerDefinition() : covenantId(0), requiredRankId(0), durationSeconds(0), cooldownSeconds(0), targetMode(CovenantPowerTargetMode::Self) {}
};

struct CovenantPowerState
{
    CovenantId covenantId;
    std::string key;
    std::int64_t activeUntil;
    std::int64_t cooldownUntil;
    std::uint32_t targetSerial;
    CovenantPowerState() : covenantId(0), activeUntil(0), cooldownUntil(0), targetSerial(0) {}
};

struct CovenantInventoryItem
{
    CovenantId covenantId;
    std::string key;
    int quantity;
    CovenantInventoryItem() : covenantId(0), quantity(0) {}
};

struct CovenantRuntimeStatus
{
    std::int64_t switchCooldownUntil;
    std::vector<CovenantId> mastered;
    std::vector<std::string> collectibles;
    CovenantRuntimeStatus() : switchCooldownUntil(0) {}
};

} }
