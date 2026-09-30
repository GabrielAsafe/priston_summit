#pragma once

#include <cstdint>

#include "gameplay/core/v2/GameplayIds.h"
#include "gameplay/skills/v2/domain/SkillDefinition.h"

namespace gameplay
{
namespace skills
{
enum class SkillRuntimeCode
{
    Ok = 0,
    InvalidRequest,
    CasterNotFound,
    CasterInactive,
    SkillNotFound
};

struct SkillRuntimeRequest
{
    EntityId caster;
    SkillId skill;
    int point;
    int visualParam;

    SkillRuntimeRequest(EntityId casterValue = EntityId(), SkillId skillValue = 0u,
                        int pointValue = 0, int visualParamValue = 0)
        : caster(casterValue), skill(skillValue), point(pointValue), visualParam(visualParamValue) {}
};

struct SkillRuntimeResult
{
    SkillRuntimeCode code;
    std::uint32_t playCode;
    std::uint32_t motionState;
    bool desiredMotionSelected;
    bool fallbackAttackMotion;

    SkillRuntimeResult()
        : code(SkillRuntimeCode::InvalidRequest), playCode(0u), motionState(0u),
          desiredMotionSelected(false), fallbackAttackMotion(false) {}

    bool Ok() const { return code == SkillRuntimeCode::Ok; }
};

class ISkillRuntime
{
public:
    virtual ~ISkillRuntime() {}
    virtual SkillRuntimeResult Begin(const SkillRuntimeRequest& request) = 0;
};
}
}
