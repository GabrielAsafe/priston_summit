#pragma once

#include <cstdint>

#include "gameplay/classes/v2/services/ClassService.h"

namespace gameplay
{
namespace classes
{
namespace legacy
{
inline bool TryLowLevelPresentItemRow(int legacyJobCode, int& out)
{
    // This row order belongs to the old rsPresentItem_LevelType_[12] tables,
    // not to class identity. Keep it here as opt-in legacy content instead of
    // polluting PlayableClassDefinition or treating JOB_CODE as an array index.
    struct Entry { int jobCode; int row; };
    static const Entry entries[] = {
        {1, 0}, {2, 1}, {3, 2}, {4, 3},
        {5, 4}, {6, 5}, {7, 6}, {8, 7}
    };

    if(!DefaultClassService().FindEnabled(legacyJobCode))
        return false;
    for(std::size_t index = 0; index < sizeof(entries) / sizeof(entries[0]); ++index)
    {
        if(entries[index].jobCode == legacyJobCode)
        {
            out = entries[index].row;
            return true;
        }
    }
    return false;
}

// Compatibility policy for the historical new-user gift flow.  It preserves
// the old content semantics: projectile/caster classes receive their class
// weapon directly; melee classes keep the legacy random weapon fallback in
// the caller.
inline std::uint32_t StarterPresentWeaponCode(int legacyJobCode, std::uint32_t lowCode)
{
    const ClassService& service = DefaultClassService();
    if(!service.FindEnabled(legacyJobCode))
        return 0u;

    if(!service.HasCapability(legacyJobCode, ClassCapability::UsesProjectile) &&
       !service.HasCapability(legacyJobCode, ClassCapability::Caster))
        return 0u;

    std::uint32_t weaponType = 0u;
    if(!service.TryDefaultWeaponType(legacyJobCode, weaponType))
        return 0u;
    return weaponType | lowCode;
}

inline std::uint32_t StarterPresentArmorCode(int legacyJobCode, std::uint32_t lowCode)
{
    const ClassService& service = DefaultClassService();
    if(!service.FindEnabled(legacyJobCode))
        return 0u;

    // sinDA1/sinDA2 families expressed at the compatibility boundary only.
    const std::uint32_t family = service.HasCapability(legacyJobCode, ClassCapability::Caster)
                                     ? 0x02050000u
                                     : 0x02010000u;
    return family | lowCode;
}
}
}
}
