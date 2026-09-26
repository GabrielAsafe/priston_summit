#pragma once

#include <string>
#include <vector>

#include "gameplay/skills/v2/domain/SkillDefinition.h"

namespace gameplay
{
namespace skills
{

class SkillRegistry
{
public:
    bool Register(const SkillDefinition& definition);
    bool RegisterAll(const std::vector<SkillDefinition>& definitions, std::string* error = 0);
    bool Validate(std::string* error = 0) const;

    const SkillDefinition* Find(SkillId id) const;
    const SkillDefinition* FindByClassAndSlot(int ownerClassId, int useSlot) const;
    std::vector<const SkillDefinition*> SkillsForClass(int ownerClassId) const;

    const std::vector<SkillDefinition>& AllSkills() const { return definitions_; }

private:
    std::vector<SkillDefinition> definitions_;
};

// Process-wide canonical registry used by both clean SkillService consumers and
// legacy adapters. The implementation is built from the legacy-backed content
// catalog, but callers do not need to depend on the content layer header.
const SkillRegistry& DefaultSkillRegistry();

}
}
