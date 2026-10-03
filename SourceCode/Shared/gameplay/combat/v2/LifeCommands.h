#pragma once

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace combat
{
enum class LifeCommandCode
{
    Ok = 0,
    InvalidRequest,
    NotFound,
    Inactive,
    InvalidState,
    WouldKill
};

struct LifeCommandResult
{
    LifeCommandCode code;
    int before;
    int after;
    int maximum;

    LifeCommandResult()
        : code(LifeCommandCode::InvalidRequest), before(0), after(0), maximum(0) {}

    bool Ok() const { return code == LifeCommandCode::Ok; }
};

class ILifeCommands
{
public:
    virtual ~ILifeCommands() {}

    virtual LifeCommandResult Restore(EntityId target, int amount) = 0;
    virtual LifeCommandResult Restore(PlayerId target, int amount) = 0;
    virtual LifeCommandResult ConsumeNonLethal(EntityId target, int amount) = 0;
    virtual LifeCommandResult ConsumeLethal(EntityId target, int amount) = 0;
    virtual LifeCommandResult Revive(EntityId target, int percentOfMaximum) = 0;
};
}
}
