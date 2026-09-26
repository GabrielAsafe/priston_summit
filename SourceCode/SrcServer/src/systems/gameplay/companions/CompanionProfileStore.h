#pragma once

#include <windows.h>

namespace gameplay
{
namespace companions
{

enum CompanionTacticPreset
{
    COMPANION_TACTIC_BALANCED = 0,
    COMPANION_TACTIC_OWNER_FOCUS = 1,
    COMPANION_TACTIC_DEFENSIVE = 2,
    COMPANION_TACTIC_HOLD_POSITION = 3
};

struct CompanionProfile
{
    CompanionProfile();

    bool loadoutActive;
    DWORD rightItemCode;
    DWORD leftItemCode;
    int mode;
    CompanionTacticPreset tactic;
    int retreatHpPercent;
    int preferredRange;
    int formationSlot;

    // Skill Loadout V2. Slots store logical UseSkill indices (1..64), not raw
    // skill codes, so they remain tied to the normalized persisted character-skill
    // contract rather than a specific DAT byte layout. Zero disables a slot.
    // Old profiles migrate with both loadouts off.
    bool skillLoadoutActive;
    BYTE activeSkillSlots[4];
    // V30 per-active-slot policy. Slot order is priority (0 highest).
    // minResourcePercent is a post-cast reserve; minimumTargets is clamped
    // to 1..16; targetMode: 0=AUTO, 1=OWNER_TARGET, 2=NEAREST.
    BYTE activeMinResourcePercent[4];
    BYTE activeMinimumTargets[4];
    BYTE activeTargetModes[4];
    bool buffLoadoutActive;
    BYTE buffSkillSlots[4];
    WORD buffIntervalSeconds[4];
    BYTE buffTargetModes[4];
};

// Profiles are intentionally separate from the character DAT. They describe
// companion-only behavior/loadout and never mutate the playable character's
// inventory or appearance record.
bool LoadCompanionProfile(const char *account, const char *characterName, CompanionProfile *profile);
bool SaveCompanionProfile(const char *account, const char *characterName, const CompanionProfile &profile);

} // namespace companions
} // namespace gameplay
