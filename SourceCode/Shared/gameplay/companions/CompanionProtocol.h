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
// Additive inventory page; all existing companion wire formats stay unchanged.
static const std::int32_t OPCODE_COMPANION_INVENTORY_PAGE = 0x48479C14;
static const std::int32_t COMPANION_INVENTORY_PROTOCOL_VERSION = 2;
// Separate trade PREVIEW transport. No commit message exists in this version.
static const std::int32_t OPCODE_COMPANION_TRADE_PAGE = 0x48479C15;
static const std::int32_t OPCODE_COMPANION_TRADE_CONFIRM = 0x48479C16;
static const std::int32_t OPCODE_COMPANION_TRADE_RESULT = 0x48479C17;
// A committed player-record image is delivered on the authenticated game
// socket. The receiver applies only the inventory, never full character state.
static const std::int32_t OPCODE_COMPANION_TRADE_DELIVERY = 0x48479C18;
static const std::int32_t OPCODE_COMPANION_TRADE_APPLY_ACK = 0x48479C19;
static const std::int32_t COMPANION_TRADE_DELIVERY_VERSION = 1;
static const std::int32_t COMPANION_TRADE_CONFIRM_VERSION = 2;
static const std::int32_t COMPANION_TRADE_PAGE_VERSION = 2;
static const int COMPANION_TRADE_PAGE_SIZE = 10;

static const int COMPANION_INVENTORY_PAGE_SIZE = 10;
static const std::int32_t COMPANION_UI_PROTOCOL_VERSION = 6;
static const int COMPANION_UI_MAX_CHARACTERS = 11;
static const int COMPANION_UI_TACTIC_RULES = 4;
static const std::int32_t COMPANION_RUNTIME_PROTOCOL_VERSION = 2;
static const int COMPANION_RUNTIME_MAX_ACTIVE = 4;
static const int COMPANION_RUNTIME_MAX_EFFECTS = 6;
// Visual v4 is the current local extension: it carries logicalSkillCode in
// addition to the legacy play code. V43 runtime-state additions use their own
// protocol version and do not require downgrading this packet.
static const std::int32_t COMPANION_VISUAL_PROTOCOL_VERSION = 4;
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
    COMPANION_UI_SET_PREFERRED_RANGE,
    COMPANION_UI_CYCLE_ACTIVE_SKILL,
    COMPANION_UI_RESET_ACTIVE_SKILLS,
    COMPANION_UI_CYCLE_BUFF_SKILL,
    COMPANION_UI_ADJUST_BUFF_INTERVAL,
    COMPANION_UI_RESET_BUFFS,
    COMPANION_UI_CYCLE_ACTIVE_MIN_RESOURCE,
    COMPANION_UI_CYCLE_ACTIVE_MIN_TARGETS,
    COMPANION_UI_CYCLE_ACTIVE_TARGET_MODE,
    COMPANION_UI_TOGGLE_CUSTOM_TACTICS,
    COMPANION_UI_TOGGLE_TACTIC_RULE,
    COMPANION_UI_CYCLE_TACTIC_CONDITION,
    COMPANION_UI_CYCLE_TACTIC_ACTION,
    COMPANION_UI_CYCLE_TACTIC_THRESHOLD,
    COMPANION_UI_MOVE_TACTIC_RULE,
    COMPANION_UI_INVENTORY_PAGE,
    COMPANION_UI_EQUIP_INVENTORY_RIGHT,
    COMPANION_UI_EQUIP_INVENTORY_LEFT,
    COMPANION_UI_SET_INVENTORY_SLOT,
    COMPANION_UI_TRADE_PAGE, // read-only; value=(side << 16)|page
    COMPANION_UI_MOVE_BAG, // value=(destinationBag << 16)|recordIndex
    COMPANION_UI_ORGANIZE_BAG, // value=bag (1 or 2)
    COMPANION_UI_MOVE_BAG_CELL // value: index[0:7] x[8:11] y[12:14] bag[15:16]
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

struct CompanionUISkillSlot
{
    std::int32_t useSlot;
    std::uint32_t skillCode;
    std::int32_t point;
    std::int32_t intervalSeconds;
    std::int32_t minResourcePercent;
    std::int32_t minimumTargets;
    std::int32_t targetMode;
    char skillName[32];
};

enum CompanionCustomTacticCondition : std::int32_t
{
    COMPANION_TACTIC_CONDITION_ALWAYS = 0,
    COMPANION_TACTIC_CONDITION_SELF_HP_BELOW,
    COMPANION_TACTIC_CONDITION_OWNER_HP_BELOW,
    COMPANION_TACTIC_CONDITION_ENEMIES_NEARBY_AT_LEAST,
    COMPANION_TACTIC_CONDITION_OWNER_HAS_TARGET,
    COMPANION_TACTIC_CONDITION_PARTY_THREATENED
};

enum CompanionCustomTacticAction : std::int32_t
{
    COMPANION_TACTIC_ACTION_ATTACK_OWNER_TARGET = 0,
    COMPANION_TACTIC_ACTION_DEFEND_PARTY,
    COMPANION_TACTIC_ACTION_ATTACK_NEAREST,
    COMPANION_TACTIC_ACTION_RETREAT,
    COMPANION_TACTIC_ACTION_HOLD_POSITION
};

struct CompanionUITacticRule
{
    std::int32_t enabled;
    std::int32_t condition;
    std::int32_t action;
    std::int32_t threshold;
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
    std::int32_t skillLoadoutActive;
    std::int32_t buffLoadoutActive;
    std::int32_t customTacticsEnabled;
    CompanionUITacticRule tacticRules[COMPANION_UI_TACTIC_RULES];
    CompanionUISkillSlot activeSkills[4];
    CompanionUISkillSlot buffSkills[4];
    CompanionUIEntry entries[COMPANION_UI_MAX_CHARACTERS];
};

enum CompanionRuntimeStatusFlags : std::uint32_t
{
    COMPANION_RUNTIME_STATUS_NONE = 0,
    COMPANION_RUNTIME_STATUS_SLOW = 1u << 0,
    COMPANION_RUNTIME_STATUS_STUN = 1u << 1,
    COMPANION_RUNTIME_STATUS_POISON = 1u << 2,
    COMPANION_RUNTIME_STATUS_DISTORTION = 1u << 3,
    COMPANION_RUNTIME_STATUS_THREAT = 1u << 4
};

struct CompanionRuntimeEffect
{
    std::uint32_t effectCode;
    std::int32_t remainingMs;
    std::int32_t positive;
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
    std::uint32_t castingSkillCode;
    std::int32_t castProgressPermille;
    std::int32_t cooldownMs;
    std::uint32_t statusFlags;
    std::int32_t effectCount;
    char castingSkillName[24];
    CompanionRuntimeEffect effects[COMPANION_RUNTIME_MAX_EFFECTS];
    char targetName[32];
};

struct CompanionInventoryItem
{
    std::int32_t recordIndex;
    std::int32_t itemPosition;
    std::uint32_t itemCode;
    std::uint32_t equipMask; // bit position: server-approved equipment slots 1..16
    std::int32_t gridX;
    std::int32_t gridY;
    std::int32_t bagNumber; // 1 primary, 2 secondary
    char itemName[32];
};

struct CompanionInventoryPagePacket
{
    std::int32_t size;
    std::int32_t code;
    std::int32_t version;
    std::int32_t status; // 1 loaded, negative means no usable record
    std::int32_t page;
    std::int32_t totalItems;
    std::int32_t itemCount;
    char characterName[32];
    CompanionInventoryItem items[COMPANION_INVENTORY_PAGE_SIZE];
};

// Side 0: authenticated owner's online identity ledger. Side 1: owned,
// offline Companion DAT. These are previews, never authoritative item grants.
struct CompanionTradePageItem
{
    std::int32_t index;
    std::int32_t position;
    std::uint32_t itemCode;
    std::uint32_t itemKey;
    std::uint32_t itemChecksum;
    char name[32];
};
struct CompanionTradePagePacket
{
    std::int32_t size;
    std::int32_t code;
    std::int32_t version;
    std::int32_t side;
    std::int32_t status;
    std::int32_t page;
    std::int32_t totalItems;
    std::int32_t itemCount;
    std::uint64_t sourceRevision;
    char characterName[32];
    CompanionTradePageItem items[COMPANION_TRADE_PAGE_SIZE];
};

// A one-item, server-verified trade request. A valid request is NOT a grant.
// The two revisions bind both displayed inventories to the player's choice.
struct CompanionTradeConfirmPacket
{
    std::int32_t size, code, version;
    std::int32_t side, itemIndex;
    std::uint32_t itemCode;
    std::uint32_t itemKey;
    std::uint32_t itemChecksum;
    std::uint64_t ownerRevision, companionRevision, requestId;
    char characterName[32];
};
// Fail-closed result. A successful validation means ONLY request eligibility;
// the current server has no complete-authority DAT/session transaction adapter.
struct CompanionTradeResultPacket
{
    std::int32_t size, code, version, status;
    std::uint64_t requestId;
    char characterName[32];
};

struct CompanionTradeApplyAckPacket
{
    std::int32_t size, code, version, status; // 1 applied, 2 fresh-save ack, negative aborted
    std::uint64_t requestId, txnId;
    char characterName[32];
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
    // Explicit skill event. CHRMOTION_STATE_SKILL by itself is insufficient:
    // player motion selection depends on AttackSkil (SKILL_PLAY_* + point).
    // skillSequence changes once per authoritative cast, even if the logical
    // motion state is identical to the previous cast.
    // Logical skill identity is independent of the legacy SKILL_PLAY_* motion/network alias.
    // This is required for Rank5 aliases (and future tiers) where multiple skills share T51..T54.
    std::uint32_t logicalSkillCode;
    std::uint32_t skillPlayCode;
    std::uint32_t skillPoint;
    std::uint32_t skillTargetSerial;
    // Snapshot the target position at authoritative cast time.  A damage cast
    // can kill/remove its target before the visual packet reaches the client;
    // keeping coordinates here lets presentation still spawn the projectile /
    // particle at the correct location instead of silently losing the effect.
    std::int32_t skillTargetX;
    std::int32_t skillTargetY;
    std::int32_t skillTargetZ;
    std::uint32_t skillMotionState;
    std::uint32_t skillSequence;
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
static_assert(sizeof(CompanionInventoryPagePacket) < 1024, "companion inventory pages must be bounded");
static_assert(sizeof(CompanionRuntimeStatePacket) < 1024, "companion runtime HUD state must remain compact");
static_assert(sizeof(CompanionVisualStatePacket) < 1024, "companion visual state must remain compact");

} // namespace protocol
} // namespace companions
} // namespace gameplay
