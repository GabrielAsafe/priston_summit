#include "gameplay/skills/v2/SkillResourceCommands.h"

#include <cassert>
#include <map>

namespace
{
class FakeSkillResourceCommands final : public gameplay::skills::ISkillResourceCommands
{
public:
    std::map<unsigned int, gameplay::skills::SkillResourceState> states;

    gameplay::skills::SkillResourceCommandResult Inspect(gameplay::EntityId caster) override
    {
        gameplay::skills::SkillResourceCommandResult result;
        std::map<unsigned int, gameplay::skills::SkillResourceState>::const_iterator it = states.find(caster.value);
        if(!caster.Valid() || it == states.end())
            return result;
        result.code = gameplay::skills::SkillResourceCommandCode::Ok;
        result.before = it->second;
        result.after = it->second;
        return result;
    }

    gameplay::skills::SkillResourceCommandResult Spend(gameplay::EntityId caster, int mana, int stamina) override
    {
        gameplay::skills::SkillResourceCommandResult result;
        std::map<unsigned int, gameplay::skills::SkillResourceState>::iterator it = states.find(caster.value);
        if(!caster.Valid() || mana < 0 || stamina < 0 || it == states.end())
            return result;
        result.before = it->second;
        result.after = it->second;
        if(it->second.mana < mana)
        {
            result.code = gameplay::skills::SkillResourceCommandCode::InsufficientMana;
            return result;
        }
        if(it->second.stamina < stamina)
        {
            result.code = gameplay::skills::SkillResourceCommandCode::InsufficientStamina;
            return result;
        }
        it->second.mana -= mana;
        it->second.stamina -= stamina;
        result.after = it->second;
        result.code = gameplay::skills::SkillResourceCommandCode::Ok;
        return result;
    }

    gameplay::skills::SkillResourceCommandResult Restore(gameplay::EntityId caster, int mana, int stamina) override
    {
        gameplay::skills::SkillResourceCommandResult result;
        std::map<unsigned int, gameplay::skills::SkillResourceState>::iterator it = states.find(caster.value);
        if(!caster.Valid() || mana < 0 || stamina < 0 || it == states.end())
            return result;
        result.before = it->second;
        it->second.mana += mana;
        if(it->second.mana > it->second.maxMana) it->second.mana = it->second.maxMana;
        it->second.stamina += stamina;
        if(it->second.stamina > it->second.maxStamina) it->second.stamina = it->second.maxStamina;
        result.after = it->second;
        result.code = gameplay::skills::SkillResourceCommandCode::Ok;
        return result;
    }

    gameplay::skills::SkillResourceCommandResult Drain(gameplay::EntityId caster, int mana, int stamina) override
    {
        gameplay::skills::SkillResourceCommandResult result;
        std::map<unsigned int, gameplay::skills::SkillResourceState>::iterator it = states.find(caster.value);
        if(!caster.Valid() || mana < 0 || stamina < 0 || it == states.end())
            return result;
        result.before = it->second;
        it->second.mana -= mana;
        if(it->second.mana < 0) it->second.mana = 0;
        it->second.stamina -= stamina;
        if(it->second.stamina < 0) it->second.stamina = 0;
        result.after = it->second;
        result.code = gameplay::skills::SkillResourceCommandCode::Ok;
        return result;
    }
};
}

int main()
{
    FakeSkillResourceCommands resources;
    resources.states[7u] = gameplay::skills::SkillResourceState(80, 100, 40, 50);

    gameplay::skills::SkillResourceCommandResult snapshot = resources.Inspect(gameplay::EntityId(7u));
    assert(snapshot.Ok());
    assert(snapshot.before.mana == 80);
    assert(snapshot.before.stamina == 40);

    gameplay::skills::SkillResourceCommandResult insufficientMana =
        resources.Spend(gameplay::EntityId(7u), 81, 1);
    assert(insufficientMana.code == gameplay::skills::SkillResourceCommandCode::InsufficientMana);
    assert(resources.states[7u].mana == 80);
    assert(resources.states[7u].stamina == 40);

    gameplay::skills::SkillResourceCommandResult insufficientStamina =
        resources.Spend(gameplay::EntityId(7u), 10, 41);
    assert(insufficientStamina.code == gameplay::skills::SkillResourceCommandCode::InsufficientStamina);
    assert(resources.states[7u].mana == 80);
    assert(resources.states[7u].stamina == 40);

    gameplay::skills::SkillResourceCommandResult spent =
        resources.Spend(gameplay::EntityId(7u), 30, 15);
    assert(spent.Ok());
    assert(spent.before.mana == 80 && spent.after.mana == 50);
    assert(spent.before.stamina == 40 && spent.after.stamina == 25);

    gameplay::skills::SkillResourceCommandResult noCost =
        resources.Spend(gameplay::EntityId(7u), 0, 0);
    assert(noCost.Ok());
    assert(noCost.after.mana == 50 && noCost.after.stamina == 25);

    gameplay::skills::SkillResourceCommandResult restored =
        resources.Restore(gameplay::EntityId(7u), 100, 10);
    assert(restored.Ok());
    assert(restored.after.mana == 100 && restored.after.stamina == 35);

    gameplay::skills::SkillResourceCommandResult drained =
        resources.Drain(gameplay::EntityId(7u), 250, 100);
    assert(drained.Ok());
    assert(drained.after.mana == 0 && drained.after.stamina == 0);
    return 0;
}
