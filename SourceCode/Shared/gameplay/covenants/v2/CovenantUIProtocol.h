#pragma once

#include <cstdint>

namespace gameplay
{
namespace covenants
{
namespace protocol
{
static const std::int32_t OPCODE_COVENANT_UI_ACTION = 0x48479D10;
static const std::int32_t OPCODE_COVENANT_UI_STATE  = 0x48479D11;
static const std::int32_t COVENANT_UI_PROTOCOL_VERSION = 3;

static const int COVENANT_UI_MAX_COVENANTS = 5;
static const int COVENANT_UI_MAX_RANKS = 3;
static const int COVENANT_UI_MAX_MODIFIERS = 8;
static const int COVENANT_UI_MAX_CHALLENGES = 3;
static const int COVENANT_UI_MAX_INVENTORY = 12;
static const int COVENANT_UI_MAX_SURVEY_AREAS = 12;
static const int COVENANT_UI_MAX_RESOURCES = 6;

enum CovenantUIAction : std::int32_t
{
    COVENANT_UI_REFRESH = 0,
    COVENANT_UI_JOIN,
    COVENANT_UI_ACTIVATE,
    COVENANT_UI_USE_POWER,
    COVENANT_UI_SET_ARTISAN_MODE,
    COVENANT_UI_REFINE_TROPHY,
    COVENANT_UI_HARVEST_MINING,
    COVENANT_UI_HARVEST_GATHERING
};

enum CovenantUIResult : std::int32_t
{
    COVENANT_UI_RESULT_NONE = 0,
    COVENANT_UI_RESULT_OK = 1,
    COVENANT_UI_RESULT_REJECTED = -1,
    COVENANT_UI_RESULT_INVALID = -2,
    COVENANT_UI_RESULT_RUNTIME_NOT_READY = -3
};

struct CovenantUIActionPacket
{
    std::int32_t size;
    std::int32_t code;
    std::int32_t version;
    std::int32_t action;
    std::int32_t covenantId;
    std::int32_t value;
    char key[64];
};

struct CovenantUIEntry
{
    std::int32_t covenantId;
    std::int32_t joined;
    std::int32_t active;
    std::int32_t mastered;
    std::int32_t rankId;
    std::int32_t reputation;
    std::int32_t nextRankReputation;
    std::int32_t canJoin;
    char key[24];
    char rankKey[32];
};

struct CovenantUIRankSummary
{
    std::int32_t rankId;
    std::int32_t minReputation;
    char key[32];
};

// Values are represented in basis points for deterministic cross-compiler wire
// format: 1000 == 10.00% for percent operations; flat values use the same
// scalar convention (10000 == 1.0).
struct CovenantUIModifier
{
    std::int32_t operation;
    std::int32_t valueBasisPoints;
    char statKey[40];
    char scopeKey[24];
};

struct CovenantUIChallenge
{
    std::int32_t targetCount;
    std::int32_t progress;
    std::int32_t reputationReward;
    std::int32_t completed;
    std::int32_t rewardClaimed;
    char key[32];
    char activityKey[32];
};

struct CovenantUIInventoryItem
{
    std::int32_t covenantId;
    std::int32_t quantity;
    char key[48];
};

struct CovenantUIResourceItem
{
    std::int32_t quantity;
    char key[48];
};

struct CovenantUISurveyArea
{
    std::int32_t area;
    std::int32_t nodeCount;
    char resourceKey[32];
};

struct CovenantUIPowerState
{
    std::int32_t available;
    std::int32_t requiredRankId;
    std::int32_t durationSeconds;
    std::int32_t cooldownSeconds;
    std::int32_t targetMode;
    std::int64_t activeUntil;
    std::int64_t cooldownUntil;
    std::uint32_t targetSerial;
    char key[32];
};

struct CovenantUIStatePacket
{
    std::int32_t size;
    std::int32_t code;
    std::int32_t version;
    std::int32_t selectedCovenantId;
    std::int32_t activeCovenantId;
    std::int32_t entryCount;
    std::int32_t rankCount;
    std::int32_t modifierCount;
    std::int32_t activeModifierCount;
    std::int32_t challengeCount;
    std::int32_t inventoryCount;
    std::int32_t surveyAreaCount;
    std::int32_t resourceCount;
    std::int32_t artisanMode;
    std::int32_t artisanMaxMode;
    std::int32_t lastResult;
    std::int64_t serverNow;
    std::int64_t switchCooldownUntil;
    char lastMessage[96];
    CovenantUIPowerState power;
    CovenantUIPowerState activePower;
    CovenantUIEntry entries[COVENANT_UI_MAX_COVENANTS];
    CovenantUIRankSummary ranks[COVENANT_UI_MAX_RANKS];
    CovenantUIModifier modifiers[COVENANT_UI_MAX_MODIFIERS];
    CovenantUIModifier activeModifiers[COVENANT_UI_MAX_MODIFIERS];
    CovenantUIChallenge challenges[COVENANT_UI_MAX_CHALLENGES];
    CovenantUIInventoryItem inventory[COVENANT_UI_MAX_INVENTORY];
    CovenantUISurveyArea surveyAreas[COVENANT_UI_MAX_SURVEY_AREAS];
    CovenantUIResourceItem resources[COVENANT_UI_MAX_RESOURCES];
};

static_assert(sizeof(CovenantUIActionPacket) <= 96, "covenant UI action packet unexpectedly large");
static_assert(sizeof(CovenantUIStatePacket) < 4096, "covenant UI state must stay below legacy socket buffer pressure thresholds");

}
}
}
