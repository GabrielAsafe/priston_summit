#include "gameplay/skills/v2/services/SkillService.h"

#include "gameplay/skills/v2/content/LegacySkillCatalog.h"

namespace gameplay
{
namespace skills
{

const SkillDefinition* SkillService::Find(SkillId id) const
{
    return registry_.Find(id);
}

const SkillDefinition* SkillService::ResolveSlot(int ownerClassId, int useSlot) const
{
    return registry_.FindByClassAndSlot(ownerClassId, useSlot);
}

std::vector<const SkillDefinition*> SkillService::SkillsForClass(int ownerClassId) const
{
    return registry_.SkillsForClass(ownerClassId);
}

bool SkillService::IsDamage(SkillId id) const
{
    const SkillDefinition* definition = Find(id);
    return definition && definition->IsExecutable() &&
           (definition->behavior == SkillBehavior::DamageSingle ||
            definition->behavior == SkillBehavior::DamageArea);
}

bool SkillService::IsControl(SkillId id) const
{
    const SkillDefinition* definition = Find(id);
    return definition && definition->IsExecutable() &&
           (definition->behavior == SkillBehavior::Control ||
            definition->behavior == SkillBehavior::DebuffSingle ||
            definition->behavior == SkillBehavior::DebuffArea);
}

bool SkillService::IsOffensive(SkillId id) const
{
    const SkillDefinition* definition = Find(id);
    return definition && definition->IsExecutable() && IsOffensiveBehavior(definition->behavior);
}

bool SkillService::IsBuff(SkillId id) const
{
    const SkillDefinition* definition = Find(id);
    return definition && definition->IsExecutable() && IsBuffBehavior(definition->behavior);
}

bool SkillService::IsSupport(SkillId id) const
{
    const SkillDefinition* definition = Find(id);
    return definition && definition->IsExecutable() && IsSupportBehavior(definition->behavior);
}

bool SkillService::IsPassive(SkillId id) const
{
    const SkillDefinition* definition = Find(id);
    return definition && definition->IsPassive();
}

std::uint32_t SkillService::LegacyPlayCode(SkillId id) const
{
    const SkillDefinition* definition = Find(id);
    return definition ? definition->legacy.playCode : 0u;
}

std::uint32_t SkillService::PresentationPlayCode(SkillId id) const
{
    const SkillDefinition* definition = Find(id);
    if(!definition) return 0u;
    return definition->legacy.presentationPlayCode
        ? definition->legacy.presentationPlayCode
        : definition->legacy.playCode;
}

bool SkillService::Validate(std::string* error) const
{
    return registry_.Validate(error);
}

const SkillService& DefaultSkillService()
{
    static const SkillService service(DefaultSkillRegistry());
    return service;
}

}
}
