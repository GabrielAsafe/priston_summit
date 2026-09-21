#pragma once

#include <cstdint>

#include "gameplay/classes/v2/services/ClassService.h"

namespace gameplay
{
namespace commerce
{
namespace legacy
{
// Compatibility item used by the old manufacture/trade preview list. The old
// code encoded this as four concrete-class branches, but the behavior is fully
// described by two stable class policies: caster armor family and gender.
inline std::uint32_t ManufacturePreviewArmorCode(int legacyJobCode)
{
    const classes::ClassService& service = classes::DefaultClassService();
    if(!service.FindEnabled(legacyJobCode))
        return 0u;

    const bool caster = service.HasCapability(legacyJobCode, classes::ClassCapability::Caster);
    classes::CharacterGender gender = classes::CharacterGender::Unspecified;
    if(!service.TryGender(legacyJobCode, gender))
        return 0u;

    const std::uint32_t armorFamily = caster ? 0x02050000u : 0x02010000u; // sinDA2 / sinDA1
    const std::uint32_t variant = gender == classes::CharacterGender::Female
                                      ? 0x00004400u  // sin52
                                      : 0x00004300u; // sin51
    return armorFamily | variant;
}
}
}
}
