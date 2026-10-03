#pragma once

#include "gameplay/skills/v2/SkillResourceCommands.h"

namespace gameplay
{
namespace skills
{
namespace legacy
{
class LegacySkillResourceAdapter final : public ISkillResourceCommands
{
public:
    SkillResourceCommandResult Inspect(EntityId caster) override;
    SkillResourceCommandResult Spend(EntityId caster, int mana, int stamina) override;
    SkillResourceCommandResult Restore(EntityId caster, int mana, int stamina) override;
    SkillResourceCommandResult Drain(EntityId caster, int mana, int stamina) override;
};

LegacySkillResourceAdapter& ServerLegacySkillResourceAdapter();
}
}
}
