#pragma once

#include <cstdint>

#include "gameplay/classes/v2/services/ClassService.h"

namespace gameplay
{
namespace classes
{
class LegacyClassAdapter
{
public:
    explicit LegacyClassAdapter(const PlayableClassRegistry& registry)
        : registry_(registry), service_(registry)
    {
    }

    bool TryJobBitCode(int legacyJobCode, std::uint32_t& value) const
    {
        return service_.TryJobBitCode(legacyJobCode, value);
    }

    bool TryBrood(int legacyJobCode, int& value) const
    {
        const PlayableClassDefinition* definition = registry_.FindByLegacyJobCode(legacyJobCode);
        if (!definition) return false;
        value = definition->brood;
        return true;
    }

    bool TryArmorModelIndex(int legacyJobCode, int& value) const
    {
        return service_.TryArmorModelIndex(legacyJobCode, value);
    }

    bool TryFormula(int legacyJobCode, CharacterFormulaProfile& value) const
    {
        return service_.TryFormula(legacyJobCode, value);
    }

    const PlayableClassDefinition* FindSelectableClass(int brood, int selectionOrder) const
    {
        return service_.FindSelectableClass(brood, selectionOrder);
    }

private:
    const PlayableClassRegistry& registry_;
    ClassService service_;
};
}
}
