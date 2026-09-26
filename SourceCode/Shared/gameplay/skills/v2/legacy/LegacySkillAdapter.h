#pragma once

#include <cstdint>

#include "gameplay/skills/v2/services/SkillService.h"

namespace gameplay
{
namespace skills
{

// Compatibility boundary for old sinSkill/SkillSub/character code. New gameplay
// code depends on SkillService; legacy code reaches the same domain through this
// adapter instead of owning a second catalog or tier-specific switch.
class LegacySkillAdapter
{
public:
    explicit LegacySkillAdapter(const SkillRegistry& registry)
        : service_(registry)
    {
    }

    const SkillDefinition* Find(std::uint32_t logicalSkillCode) const
    {
        return service_.Find(logicalSkillCode);
    }

    const SkillDefinition* ResolveSlot(int legacyJobCode, int useSlot) const
    {
        return service_.ResolveSlot(legacyJobCode, useSlot);
    }

    std::vector<const SkillDefinition*> SkillsForClass(int legacyJobCode) const
    {
        return service_.SkillsForClass(legacyJobCode);
    }

    std::uint32_t PlayCode(std::uint32_t logicalSkillCode) const
    {
        return service_.LegacyPlayCode(logicalSkillCode);
    }

    std::uint32_t PresentationPlayCode(std::uint32_t logicalSkillCode) const
    {
        return service_.PresentationPlayCode(logicalSkillCode);
    }

    bool IsDamage(std::uint32_t logicalSkillCode) const
    {
        return service_.IsDamage(logicalSkillCode);
    }

    bool IsControl(std::uint32_t logicalSkillCode) const
    {
        return service_.IsControl(logicalSkillCode);
    }

    bool IsOffensive(std::uint32_t logicalSkillCode) const
    {
        return service_.IsOffensive(logicalSkillCode);
    }

    bool IsBuff(std::uint32_t logicalSkillCode) const
    {
        if(service_.IsBuff(logicalSkillCode))
            return true;

        // The old runtime implements these summon identities through the same
        // timed-effect path as buffs. Keep that quirk at the legacy boundary;
        // the clean domain still describes them as Summon.
        return logicalSkillCode == 0x04020002u || // Falcon
               logicalSkillCode == 0x04040002u || // Golden Falcon
               logicalSkillCode == 0x07080004u || // Summon Muspell
               logicalSkillCode == 0x08040004u;   // Dancing Sword
    }

    bool IsSupport(std::uint32_t logicalSkillCode) const
    {
        return service_.IsSupport(logicalSkillCode);
    }

private:
    SkillService service_;
};

inline const LegacySkillAdapter& DefaultLegacySkillAdapter()
{
    static const LegacySkillAdapter adapter(DefaultSkillRegistry());
    return adapter;
}

}
}
