#pragma once

#include <vector>

#include "gameplay/skills/v2/SkillRegistry.h"

namespace gameplay
{
namespace skills
{

class SkillService
{
public:
    explicit SkillService(const SkillRegistry& registry) : registry_(registry) {}

    const SkillDefinition* Find(SkillId id) const;
    const SkillDefinition* ResolveSlot(int ownerClassId, int useSlot) const;
    std::vector<const SkillDefinition*> SkillsForClass(int ownerClassId) const;

    bool IsDamage(SkillId id) const;
    bool IsControl(SkillId id) const;
    bool IsOffensive(SkillId id) const;
    bool IsBuff(SkillId id) const;
    bool IsSupport(SkillId id) const;
    bool IsPassive(SkillId id) const;

    std::uint32_t LegacyPlayCode(SkillId id) const;
    std::uint32_t PresentationPlayCode(SkillId id) const;

    bool Validate(std::string* error = 0) const;

private:
    const SkillRegistry& registry_;
};

const SkillService& DefaultSkillService();

}
}
