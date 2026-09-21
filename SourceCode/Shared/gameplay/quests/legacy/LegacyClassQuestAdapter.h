#pragma once

#include <cstdint>

#include "gameplay/classes/v2/services/ClassService.h"

namespace gameplay
{
namespace quests
{
namespace legacy
{
inline bool SupportsLegacyChangeJobQuestContent(int legacyJobCode)
{
    // The historical rank-up quest scripts/rewards exist only for these eight
    // classes. This is opt-in compatibility content, not a property inferred
    // from JOB_CODE ranges or from the mere existence of a rank chain.
    static const int supportedJobs[] = {1, 2, 3, 4, 5, 6, 7, 8};
    if(!classes::DefaultClassService().FindEnabled(legacyJobCode))
        return false;
    for(std::size_t i = 0; i < sizeof(supportedJobs) / sizeof(supportedJobs[0]); ++i)
        if(supportedJobs[i] == legacyJobCode) return true;
    return false;
}

inline bool SupportsLegacyChangeJobQuestStep(int legacyJobCode, int currentChangeJob)
{
    if(!SupportsLegacyChangeJobQuestContent(legacyJobCode))
        return false;

    // This subsystem owns only the historical 20/40/60 promotions. Tier 5 is
    // a separate QuestDB-backed flow and future ranks must provide their own
    // content integration rather than silently reusing these scripts.
    if(currentChangeJob < 0 || currentChangeJob > 2)
        return false;

    int nextChangeJob = currentChangeJob;
    return classes::DefaultClassService().TryNextLegacyChangeJob(
        legacyJobCode, currentChangeJob, nextChangeJob);
}

// Compatibility policy for old quest content. The quest subsystem consumes
// stable class metadata instead of enumerating concrete JOBCODE_* values.
inline std::uint32_t DefaultWeaponRewardCode(int legacyJobCode, std::uint32_t lowCode)
{
    std::uint32_t weaponType = 0u;
    if(!classes::DefaultClassService().TryDefaultWeaponType(legacyJobCode, weaponType))
        return 0u;
    return weaponType | lowCode;
}

inline bool UsesCasterStarterDefense(int legacyJobCode)
{
    return classes::DefaultClassService().HasCapability(
        legacyJobCode, classes::ClassCapability::Caster);
}

inline int QuestBoardPresentationGroup(int legacyJobCode)
{
    const classes::ClassService& service = classes::DefaultClassService();
    if(service.HasCapability(legacyJobCode, classes::ClassCapability::Caster))
        return 11;
    if(service.HasCapability(legacyJobCode, classes::ClassCapability::UsesProjectile))
        return 8;
    return service.FindEnabled(legacyJobCode) ? 5 : 0;
}

inline std::uint32_t ChangeJob3WeaponLowCode(std::uint32_t weaponType)
{
    switch(weaponType)
    {
    case 0x01010000u: return 0x00000800u; // Axe
    case 0x01020000u: return 0x00000800u; // Claw
    case 0x01060000u: return 0x00000A00u; // Bow
    case 0x01050000u: return 0x00000900u; // Pike
    case 0x01080000u: return 0x00000900u; // Javelin
    case 0x01070000u: return 0x00000A00u; // Sword
    case 0x01040000u: return 0x00000900u; // Magic
    default: return 0u;
    }
}

inline std::uint32_t ChangeJob3WeaponRewardCode(int legacyJobCode)
{
    std::uint32_t weaponType = 0u;
    if(!classes::DefaultClassService().TryDefaultWeaponType(legacyJobCode, weaponType))
        return 0u;
    const std::uint32_t lowCode = ChangeJob3WeaponLowCode(weaponType);
    return lowCode != 0u ? (weaponType | lowCode) : 0u;
}

inline std::uint32_t ChangeJob3OrbRewardCode(int legacyJobCode)
{
    const classes::ClassService& service = classes::DefaultClassService();
    if(!service.FindEnabled(legacyJobCode))
        return 0u;

    // sinOR2 family. The old quest groups projectile classes together, casters
    // together, and gives the remaining melee classes the base variant.
    const std::uint32_t orbFamily = 0x03050000u;
    if(service.HasCapability(legacyJobCode, classes::ClassCapability::Caster))
        return orbFamily | 0x00000400u;
    if(service.HasCapability(legacyJobCode, classes::ClassCapability::UsesProjectile))
        return orbFamily | 0x00000300u;
    return orbFamily | 0x00000200u;
}

inline int ChangeJob3ItemNameIndex(int legacyJobCode)
{
    std::uint32_t weaponType = 0u;
    if(!classes::DefaultClassService().TryDefaultWeaponType(legacyJobCode, weaponType))
        return -1;
    switch(weaponType)
    {
    case 0x01010000u: return 0; // Axe / Fighter
    case 0x01020000u: return 1; // Claw / Mechanician
    case 0x01060000u: return 2; // Bow / Archer
    case 0x01050000u: return 3; // Pike / Pikeman
    case 0x01080000u: return 4; // Javelin / Atalanta
    case 0x01070000u: return 5; // Sword / Knight
    case 0x01040000u: return 6; // Magic / caster classes
    default: return -1;
    }
}

inline bool UsesFieldBasedChangeJob3Progress(int legacyJobCode)
{
    return classes::DefaultClassService().HasCapability(
        legacyJobCode, classes::ClassCapability::Caster);
}

inline std::uint32_t ChangeJobPromotionTokenCode(int legacyJobCode)
{
    const classes::ClassService& service = classes::DefaultClassService();
    if(!service.FindEnabled(legacyJobCode))
        return 0u;

    // Historical rsCheck_ChageJob uses a different OR2 variant mapping than
    // the lost-item flow. Preserve that content contract while deriving the
    // class grouping from stable metadata.
    const std::uint32_t orbFamily = 0x03050000u; // sinOR2
    if(service.HasCapability(legacyJobCode, classes::ClassCapability::Caster))
        return orbFamily | 0x00000200u; // sin02
    if(service.HasCapability(legacyJobCode, classes::ClassCapability::UsesProjectile))
        return orbFamily | 0x00000400u; // sin04
    return orbFamily | 0x00000300u; // sin03
}

}
}
}
