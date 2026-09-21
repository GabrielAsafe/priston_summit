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
};

// Profiles are intentionally separate from the character DAT. They describe
// companion-only behavior/loadout and never mutate the playable character's
// inventory or appearance record.
bool LoadCompanionProfile(const char *account, const char *characterName, CompanionProfile *profile);
bool SaveCompanionProfile(const char *account, const char *characterName, const CompanionProfile &profile);

} // namespace companions
} // namespace gameplay
