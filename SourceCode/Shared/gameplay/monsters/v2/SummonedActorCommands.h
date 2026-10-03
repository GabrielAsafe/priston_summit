#pragma once

#include <cstdint>

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace monsters
{
enum class SummonedActorCommandCode
{
    Ok = 0,
    InvalidRequest,
    NotFound,
    Inactive
};

struct SummonedActorProfile
{
    std::uint32_t expiresAtMs;
    int attackDamageMin;
    int attackDamageMax;
    int attackRating;
    int life;
    int moveStepPrimary;
    int moveStepSecondary;
    bool overrideDefence;
    int defence;
    bool overridePlayClass;
    int playClassPhase;
    bool overrideVisualSkill;
    int visualSkillCode;
    int visualSkillMode;

    SummonedActorProfile()
        : expiresAtMs(0u), attackDamageMin(0), attackDamageMax(0), attackRating(0), life(1),
          moveStepPrimary(1), moveStepSecondary(2), overrideDefence(false), defence(0),
          overridePlayClass(false), playClassPhase(0), overrideVisualSkill(false),
          visualSkillCode(0), visualSkillMode(0) {}
};

struct SummonedActorConfigureRequest
{
    EntityId actor;
    SummonedActorProfile profile;

    SummonedActorConfigureRequest(EntityId actorValue = EntityId(),
                                  const SummonedActorProfile& profileValue = SummonedActorProfile())
        : actor(actorValue), profile(profileValue) {}
};

struct SummonedActorConfigureResult
{
    SummonedActorCommandCode code;
    EntityId actor;
    int appliedLife;

    SummonedActorConfigureResult()
        : code(SummonedActorCommandCode::InvalidRequest), actor(), appliedLife(0) {}
    bool Ok() const { return code == SummonedActorCommandCode::Ok; }
};

class ISummonedActorCommands
{
public:
    virtual ~ISummonedActorCommands() {}
    virtual SummonedActorConfigureResult Configure(const SummonedActorConfigureRequest& request) = 0;
};
}
}
