#pragma once

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace skills
{
enum class SkillResourceCommandCode
{
    Ok = 0,
    InvalidRequest,
    NotFound,
    Inactive,
    InsufficientMana,
    InsufficientStamina
};

struct SkillResourceState
{
    int mana;
    int maxMana;
    int stamina;
    int maxStamina;

    SkillResourceState(int manaValue = 0, int maxManaValue = 0,
                       int staminaValue = 0, int maxStaminaValue = 0)
        : mana(manaValue), maxMana(maxManaValue),
          stamina(staminaValue), maxStamina(maxStaminaValue) {}
};

struct SkillResourceCommandResult
{
    SkillResourceCommandCode code;
    SkillResourceState before;
    SkillResourceState after;

    SkillResourceCommandResult()
        : code(SkillResourceCommandCode::InvalidRequest), before(), after() {}

    bool Ok() const { return code == SkillResourceCommandCode::Ok; }
};

class ISkillResourceCommands
{
public:
    virtual ~ISkillResourceCommands() {}

    virtual SkillResourceCommandResult Inspect(EntityId caster) = 0;
    virtual SkillResourceCommandResult Spend(EntityId caster, int mana, int stamina) = 0;
    virtual SkillResourceCommandResult Restore(EntityId caster, int mana, int stamina) = 0;
    virtual SkillResourceCommandResult Drain(EntityId caster, int mana, int stamina) = 0;
};
}
}
