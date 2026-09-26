#pragma once

#include <array>
#include <cstdint>
#include <string>

namespace ui::v2
{
constexpr std::size_t CovenantUiMaxCovenants = 5;
constexpr std::size_t CovenantUiMaxRanks = 3;
constexpr std::size_t CovenantUiMaxModifiers = 8;
constexpr std::size_t CovenantUiMaxChallenges = 3;
constexpr std::size_t CovenantUiMaxInventory = 12;
constexpr std::size_t CovenantUiMaxSurveyAreas = 12;
constexpr std::size_t CovenantUiMaxResources = 6;

enum class CovenantUiAction
{
    Refresh,
    Join,
    Activate,
    UsePower,
    SetArtisanMode,
    RefineTrophy,
    HarvestMining,
    HarvestGathering
};

struct CovenantUiEntry
{
    int covenantId = 0;
    bool joined = false;
    bool active = false;
    bool mastered = false;
    int rankId = 0;
    int reputation = 0;
    int nextRankReputation = 0;
    bool canJoin = false;
    std::string key;
    std::string rankKey;
};

struct CovenantUiRankSummary
{
    int rankId = 0;
    int minReputation = 0;
    std::string key;
};

struct CovenantUiModifier
{
    int operation = 0;
    int valueBasisPoints = 0;
    std::string statKey;
    std::string scopeKey;
};

struct CovenantUiChallenge
{
    int targetCount = 0;
    int progress = 0;
    int reputationReward = 0;
    bool completed = false;
    bool rewardClaimed = false;
    std::string key;
    std::string activityKey;
};

struct CovenantUiInventoryItem
{
    int covenantId = 0;
    int quantity = 0;
    std::string key;
};

struct CovenantUiResourceItem
{
    int quantity = 0;
    std::string key;
};

struct CovenantUiSurveyArea
{
    int area = 0;
    int nodeCount = 0;
    std::string resourceKey;
};

struct CovenantUiPowerState
{
    bool available = false;
    int requiredRankId = 0;
    int durationSeconds = 0;
    int cooldownSeconds = 0;
    int targetMode = 0;
    std::int64_t activeUntil = 0;
    std::int64_t cooldownUntil = 0;
    std::uint32_t targetSerial = 0;
    std::string key;
};

struct CovenantUiSnapshot
{
    bool valid = false;
    int selectedCovenantId = 1;
    int activeCovenantId = 0;
    int entryCount = 0;
    int rankCount = 0;
    int modifierCount = 0;
    int activeModifierCount = 0;
    int challengeCount = 0;
    int inventoryCount = 0;
    int surveyAreaCount = 0;
    int resourceCount = 0;
    int artisanMode = 0;
    int artisanMaxMode = 0;
    int lastResult = 0;
    std::int64_t serverNow = 0;
    std::int64_t switchCooldownUntil = 0;
    std::string lastMessage;
    CovenantUiPowerState power;
    CovenantUiPowerState activePower;
    std::array<CovenantUiEntry, CovenantUiMaxCovenants> entries{};
    std::array<CovenantUiRankSummary, CovenantUiMaxRanks> ranks{};
    std::array<CovenantUiModifier, CovenantUiMaxModifiers> modifiers{};
    std::array<CovenantUiModifier, CovenantUiMaxModifiers> activeModifiers{};
    std::array<CovenantUiChallenge, CovenantUiMaxChallenges> challenges{};
    std::array<CovenantUiInventoryItem, CovenantUiMaxInventory> inventory{};
    std::array<CovenantUiSurveyArea, CovenantUiMaxSurveyAreas> surveyAreas{};
    std::array<CovenantUiResourceItem, CovenantUiMaxResources> resources{};
};
}
