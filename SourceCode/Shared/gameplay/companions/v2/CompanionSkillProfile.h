#pragma once

#include "gameplay/classes/v2/domain/ClassDefinition.h"
#include "gameplay/skills/v2/domain/SkillDefinition.h"

namespace gameplay
{
namespace companions
{

struct CompanionClassSkillProfile
{
    classes::PlayableClassId classId;
    skills::SkillId defaultDamageSkill;
    bool healingRole;
};

inline const CompanionClassSkillProfile* FindCompanionClassSkillProfile(
    classes::PlayableClassId classId)
{
    // Exact parity with the legacy Companion class switch. Skill identities are
    // logical SkillIds from the canonical skill catalog, not wire play codes.
    static const CompanionClassSkillProfile profiles[] = {
        {1, 0x02010004u, false}, // Fighter     -> Impact
        {2, 0x01020001u, false}, // Mechanician -> Great Smash
        {3, 0x04010003u, false}, // Archer      -> Wind Arrow
        {4, 0x03010004u, false}, // Pikeman     -> Jumping Crash
        {5, 0x06020002u, false}, // Atalanta    -> Twist Javelin
        {6, 0x05010001u, false}, // Knight      -> Sword Blast
        {7, 0x08010002u, false}, // Magician    -> Fire Bolt
        {8, 0x07010002u, true},  // Priestess   -> Holy Bolt + healing role
    };

    for(const CompanionClassSkillProfile& profile : profiles)
        if(profile.classId == classId)
            return &profile;
    return 0;
}

inline skills::SkillId DefaultDamageSkillForClass(classes::PlayableClassId classId)
{
    const CompanionClassSkillProfile* profile = FindCompanionClassSkillProfile(classId);
    return profile ? profile->defaultDamageSkill : 0u;
}

inline bool HasCompanionHealingRole(classes::PlayableClassId classId)
{
    const CompanionClassSkillProfile* profile = FindCompanionClassSkillProfile(classId);
    return profile && profile->healingRole;
}

}
}
