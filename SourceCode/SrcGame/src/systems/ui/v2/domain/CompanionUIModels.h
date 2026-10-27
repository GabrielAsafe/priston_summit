#pragma once

#include <array>
#include <cstdint>
#include <string>

namespace ui::v2
{
constexpr std::size_t CompanionUiMaxCharacters = 11;

enum class CompanionUiMode
{
    Aggressive = 0,
    Follow = 1,
    Passive = 2
};

enum class CompanionUiTactic
{
    Balanced = 0,
    OwnerFocus = 1,
    Defensive = 2,
    HoldPosition = 3
};

enum class CompanionUiAction
{
    Refresh,
    Select,
    Summon,
    Dismiss,
    DismissAll,
    SetMode,
    EquipRightNext,
    EquipRightPrev,
    EquipLeftNext,
    EquipLeftPrev,
    ResetEquipment,
    ResyncData,
    SetTactic,
    SetRetreatHp,
    SetPreferredRange,
    CycleActiveSkill,
    ResetActiveSkills,
    CycleBuffSkill,
    AdjustBuffInterval,
    ResetBuffs,
    CycleActiveMinResource,
    CycleActiveMinTargets,
    CycleActiveTargetMode,
    ToggleCustomTactics,
    ToggleTacticRule,
    CycleTacticCondition,
    CycleTacticAction,
    CycleTacticThreshold,
    MoveTacticRule,
    InventoryPage,
    EquipInventoryRight,
    EquipInventoryLeft,
    SetInventorySlot,
    TradePage,
    MoveBag,
    OrganizeBag,
    MoveBagCell
};

struct CompanionUiEntry
{
    std::string name;
    int level = 0;
    int jobCode = 0;
    bool active = false;
    bool selected = false;
    bool currentCharacter = false;
    std::uint32_t rightItemCode = 0;
    std::uint32_t leftItemCode = 0;
    std::string rightItemName;
    std::string leftItemName;
    int life = 0;
    int maxLife = 0;
    int mana = 0;
    int maxMana = 0;
    bool dead = false;
    std::string targetName;
    CompanionUiMode mode = CompanionUiMode::Aggressive;
    CompanionUiTactic tactic = CompanionUiTactic::Balanced;
    int retreatHpPercent = 0;
    int preferredRange = 0;
};

struct CompanionUiTacticRule
{
    bool enabled = false;
    int condition = 0;
    int action = 0;
    int threshold = 0;
};

struct CompanionUiSkillSlot
{
    int useSlot = 0;
    std::uint32_t skillCode = 0;
    int point = 0;
    int intervalSeconds = 0;
    int minResourcePercent = 0;
    int minimumTargets = 1;
    int targetMode = 0;
    std::string name;
};

struct CompanionUiSnapshot
{
    bool valid = false;
    CompanionUiMode mode = CompanionUiMode::Aggressive;
    int activeCount = 0;
    int maxActive = 4;
    int entryCount = 0;
    int lastResult = 0;
    bool skillLoadoutActive = false;
    bool buffLoadoutActive = false;
    bool customTacticsEnabled = false;
    std::array<CompanionUiTacticRule, 4> tacticRules{};
    std::array<CompanionUiSkillSlot, 4> activeSkills{};
    std::array<CompanionUiSkillSlot, 4> buffSkills{};
    std::array<CompanionUiEntry, CompanionUiMaxCharacters> entries{};
};

struct CompanionInventoryItemSnapshot
{
    int recordIndex = -1;
    int position = 0;
    std::uint32_t code = 0;
    std::uint32_t equipMask = 0;
    int gridX = -1;
    int gridY = -1;
    int bagNumber = 1;
    std::string name;
};

struct CompanionInventorySnapshot
{
    bool received = false;
    int status = 0;
    int page = 0;
    int totalItems = 0;
    int itemCount = 0;
    std::string characterName;
    std::array<CompanionInventoryItemSnapshot, 10> items{};
};

struct CompanionRuntimeEffectSnapshot
{
    std::uint32_t effectCode = 0;
    std::string name;
    int remainingMs = 0;
    bool positive = true;
};

struct CompanionRuntimeEntrySnapshot
{
    std::string name;
    std::uint32_t objectSerial = 0;
    int level = 0;
    int jobCode = 0;
    int life = 0;
    int maxLife = 0;
    int mana = 0;
    int maxMana = 0;
    bool dead = false;
    CompanionUiMode mode = CompanionUiMode::Aggressive;
    CompanionUiTactic tactic = CompanionUiTactic::Balanced;
    bool casting = false;
    std::uint32_t castingSkillCode = 0;
    std::string castingSkillName;
    int castProgressPermille = 0;
    int cooldownMs = 0;
    std::uint32_t statusFlags = 0;
    int effectCount = 0;
    std::array<CompanionRuntimeEffectSnapshot, 6> effects{};
    std::string targetName;
};

struct CompanionTradePreviewItem
{
    int index = -1;
    int position = 0;
    std::uint32_t code = 0;
    std::uint32_t key = 0;
    std::uint32_t checksum = 0;
    std::string name;
};
struct CompanionTradePreviewPage
{
    bool received = false;
    int status = 0;
    int side = -1;
    int page = 0;
    int totalItems = 0;
    int itemCount = 0;
    std::uint64_t revision = 0;
    std::string characterName;
    std::array<CompanionTradePreviewItem, 10> items{};
};

struct CompanionTradeAttemptResult
{
    bool received = false;
    std::int32_t status = 0;
    std::uint64_t requestId = 0;
    std::string characterName;
};

struct CompanionRuntimeSnapshot
{
    bool valid = false;
    int entryCount = 0;
    std::array<CompanionRuntimeEntrySnapshot, 4> entries{};
};
}
