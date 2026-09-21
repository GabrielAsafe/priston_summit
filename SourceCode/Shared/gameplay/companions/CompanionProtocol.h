#pragma once

#include <cstdint>

namespace gameplay
{
namespace companions
{
namespace protocol
{

static const std::int32_t OPCODE_COMPANION_UI_ACTION = 0x48479C10;
static const std::int32_t OPCODE_COMPANION_UI_STATE  = 0x48479C11;
static const std::int32_t OPCODE_COMPANION_RUNTIME_STATE = 0x48479C12;
static const std::int32_t OPCODE_COMPANION_VISUAL_STATE = 0x48479C13;
static const std::int32_t COMPANION_UI_PROTOCOL_VERSION = 3;
static const int COMPANION_UI_MAX_CHARACTERS = 11;
static const std::int32_t COMPANION_RUNTIME_PROTOCOL_VERSION = 1;
static const int COMPANION_RUNTIME_MAX_ACTIVE = 4;
static const std::int32_t COMPANION_VISUAL_PROTOCOL_VERSION = 1;
static const int COMPANION_VISUAL_MAX_ACTIVE = 4;

enum CompanionUIAction : std::int32_t
{
    COMPANION_UI_REFRESH = 0,
    COMPANION_UI_SELECT,
    COMPANION_UI_SUMMON,
    COMPANION_UI_DISMISS,
    COMPANION_UI_DISMISS_ALL,
    COMPANION_UI_SET_MODE,
    COMPANION_UI_EQUIP_RIGHT_NEXT,
    COMPANION_UI_EQUIP_RIGHT_PREV,
    COMPANION_UI_EQUIP_LEFT_NEXT,
    COMPANION_UI_EQUIP_LEFT_PREV,
    COMPANION_UI_RESET_EQUIPMENT,
    COMPANION_UI_RESYNC_DAT,
    COMPANION_UI_SET_TACTIC,
    COMPANION_UI_SET_RETREAT_HP,
    COMPANION_UI_SET_PREFERRED_RANGE
};

struct CompanionUIActionPacket
{
    std::int32_t size;
    std::int32_t code;
    std::int32_t version;
    std::int32_t action;
    std::int32_t value;
    char characterName[32];
};

struct CompanionUIEntry
{
    char characterName[32];
    std::int32_t level;
    std::int32_t jobCode;
    std::int32_t active;
    std::int32_t selected;
    std::int32_t currentCharacter;
    std::uint32_t rightItemCode;
    std::uint32_t leftItemCode;
    char rightItemName[32];
    char leftItemName[32];
    std::int32_t life;
    std::int32_t maxLife;
    std::int32_t mana;
    std::int32_t maxMana;
    std::int32_t dead;
    char targetName[32];
    std::int32_t mode;
    std::int32_t tactic;
    std::int32_t retreatHpPercent;
    std::int32_t preferredRange;
};

struct CompanionUIStatePacket
{
    std::int32_t size;
    std::int32_t code;
    std::int32_t version;
    std::int32_t mode;
    std::int32_t activeCount;
    std::int32_t maxActive;
    std::int32_t entryCount;
    std::int32_t lastResult;
    CompanionUIEntry entries[COMPANION_UI_MAX_CHARACTERS];
};

struct CompanionRuntimeEntry
{
    char characterName[32];
    std::uint32_t objectSerial;
    std::int32_t level;
    std::int32_t jobCode;
    std::int32_t life;
    std::int32_t maxLife;
    std::int32_t mana;
    std::int32_t maxMana;
    std::int32_t dead;
    std::int32_t mode;
    std::int32_t tactic;
    std::int32_t casting;
    char targetName[32];
};

struct CompanionRuntimeStatePacket
{
    std::int32_t size;
    std::int32_t code;
    std::int32_t version;
    std::int32_t entryCount;
    CompanionRuntimeEntry entries[COMPANION_RUNTIME_MAX_ACTIVE];
};

enum CompanionVisualFlags : std::uint32_t
{
    COMPANION_VISUAL_FLAG_NONE = 0,
    COMPANION_VISUAL_FLAG_DEAD = 1u << 0
};

// Authoritative visual state for account companions. Unlike PLAYBUFF, every
// field here is stable across independent server/client model corpora: world
// coordinates, logical CHRMOTION_STATE_*, canonical item CODEs and explicit
// monotonically increasing revisions/sequences. Model-local action indices and
// animation frame numbers are deliberately not transported.
struct CompanionVisualEntry
{
    char characterName[32];
    std::uint32_t objectSerial;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::int32_t angleY;
    std::uint32_t motionState;
    std::uint32_t motionSequence;
    std::int32_t life;
    std::int32_t maxLife;
    std::uint32_t rightItemCode;
    std::uint32_t leftItemCode;
    std::uint32_t equipmentRevision;
    std::uint32_t flags;
};

struct CompanionVisualStatePacket
{
    std::int32_t size;
    std::int32_t code;
    std::int32_t version;
    std::int32_t entryCount;
    CompanionVisualEntry entries[COMPANION_VISUAL_MAX_ACTIVE];
};

static_assert(sizeof(CompanionUIActionPacket) <= 64, "companion action packet unexpectedly large");
static_assert(sizeof(CompanionUIStatePacket) < 4096, "companion UI state must stay below legacy socket buffer pressure thresholds");
static_assert(sizeof(CompanionRuntimeStatePacket) < 1024, "companion runtime HUD state must remain compact");
static_assert(sizeof(CompanionVisualStatePacket) < 1024, "companion visual state must remain compact");

} // namespace protocol
} // namespace companions
} // namespace gameplay
