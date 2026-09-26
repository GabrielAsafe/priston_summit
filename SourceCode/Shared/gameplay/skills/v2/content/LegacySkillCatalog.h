#pragma once

#include <string>
#include <vector>

#include "gameplay/skills/v2/domain/SkillDefinition.h"

namespace gameplay
{
namespace skills
{

std::vector<SkillDefinition> LegacySkillCatalog();
bool BuildDefaultSkillRegistry(class SkillRegistry& registry, std::string* error = 0);
const SkillRegistry& DefaultSkillRegistry();

}
}
