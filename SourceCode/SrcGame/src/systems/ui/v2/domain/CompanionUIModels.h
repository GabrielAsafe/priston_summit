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
    SetPreferredRange
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

struct CompanionUiSnapshot
{
    bool valid = false;
    CompanionUiMode mode = CompanionUiMode::Aggressive;
    int activeCount = 0;
    int maxActive = 4;
    int entryCount = 0;
    int lastResult = 0;
    std::array<CompanionUiEntry, CompanionUiMaxCharacters> entries{};
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
    std::string targetName;
};

struct CompanionRuntimeSnapshot
{
    bool valid = false;
    int entryCount = 0;
    std::array<CompanionRuntimeEntrySnapshot, 4> entries{};
};
}
