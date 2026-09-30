#pragma once

#include "gameplay/skills/v2/runtime/SkillRuntime.h"

namespace gameplay
{
namespace skills
{
namespace legacy
{
class LegacySkillRuntimeAdapter final : public ISkillRuntime
{
public:
    SkillRuntimeResult Begin(const SkillRuntimeRequest& request) override;
};

LegacySkillRuntimeAdapter& ServerLegacySkillRuntimeAdapter();
}
}
}
