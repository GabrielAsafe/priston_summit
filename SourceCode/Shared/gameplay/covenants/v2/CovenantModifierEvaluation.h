#pragma once

#include <string>
#include <vector>

#include "gameplay/modifiers/v2/GameplayModifier.h"

namespace gameplay
{
namespace covenants
{
struct CovenantModifierAggregate
{
    bool matched;
    double flat;
    double additivePercent;
    double multiplicativeFactor;

    CovenantModifierAggregate()
        : matched(false), flat(0.0), additivePercent(0.0), multiplicativeFactor(1.0)
    {
    }
};

inline CovenantModifierAggregate AggregateCovenantModifiers(
    const std::vector<modifiers::GameplayModifier>& modifiers,
    const std::string& statKey,
    const std::string& scopeKey,
    bool includeGlobalScope = true)
{
    CovenantModifierAggregate result;
    for (std::vector<modifiers::GameplayModifier>::const_iterator it = modifiers.begin();
         it != modifiers.end(); ++it)
    {
        if (it->statKey != statKey)
            continue;
        const bool exactScope = it->scopeKey == scopeKey;
        const bool globalScope = includeGlobalScope && it->scopeKey == "global";
        const bool unscoped = it->scopeKey.empty();
        if (!exactScope && !globalScope && !unscoped)
            continue;

        result.matched = true;
        switch (it->operation)
        {
        case modifiers::ModifierOperation::Flat:
            result.flat += it->value;
            break;
        case modifiers::ModifierOperation::AdditivePercent:
            result.additivePercent += it->value;
            break;
        case modifiers::ModifierOperation::MultiplicativePercent:
            result.multiplicativeFactor *= (1.0 + it->value);
            break;
        }
    }
    return result;
}

inline double ApplyCovenantModifiers(double baseValue,
                                     const CovenantModifierAggregate& aggregate)
{
    return (baseValue + aggregate.flat) *
           (1.0 + aggregate.additivePercent) *
           aggregate.multiplicativeFactor;
}

inline bool CovenantFlagEnabled(const CovenantModifierAggregate& aggregate)
{
    return aggregate.matched && aggregate.flat > 0.0;
}
}
}
