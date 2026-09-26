#include <cassert>
#include <cmath>
#include <vector>

#include "gameplay/covenants/v2/CovenantModifierEvaluation.h"

int main()
{
    using namespace gameplay;
    std::vector<modifiers::GameplayModifier> values;

    modifiers::GameplayModifier local;
    local.statKey = "monster.drop_chance";
    local.scopeKey = "monster";
    local.operation = modifiers::ModifierOperation::AdditivePercent;
    local.value = 0.10;
    values.push_back(local);

    modifiers::GameplayModifier global;
    global.statKey = "monster.drop_chance";
    global.scopeKey = "global";
    global.operation = modifiers::ModifierOperation::AdditivePercent;
    global.value = 0.05;
    values.push_back(global);

    modifiers::GameplayModifier flag;
    flag.statKey = "feature.enabled";
    flag.scopeKey = "feature";
    flag.operation = modifiers::ModifierOperation::Flat;
    flag.value = 1.0;
    values.push_back(flag);

    const covenants::CovenantModifierAggregate drop =
        covenants::AggregateCovenantModifiers(values, "monster.drop_chance", "monster");
    assert(drop.matched);
    assert(std::fabs(drop.additivePercent - 0.15) < 0.000001);
    assert(std::fabs(covenants::ApplyCovenantModifiers(100.0, drop) - 115.0) < 0.000001);

    const covenants::CovenantModifierAggregate enabled =
        covenants::AggregateCovenantModifiers(values, "feature.enabled", "feature");
    assert(covenants::CovenantFlagEnabled(enabled));
    return 0;
}
