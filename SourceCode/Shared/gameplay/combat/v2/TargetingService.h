#pragma once

#include <cstdint>

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace combat
{
enum class TargetingCode
{
    Ok = 0,
    InvalidRequest,
    ActorNotFound,
    TargetNotFound,
    ActorInactive,
    TargetInactive
};

enum class TargetReleaseMode
{
    AcquiredOnly = 0,
    AcquiredAndForced
};

struct TargetingResult
{
    TargetingCode code;
    bool changed;

    TargetingResult() : code(TargetingCode::InvalidRequest), changed(false) {}
    bool Ok() const { return code == TargetingCode::Ok; }
};

class ITargetingService
{
public:
    virtual ~ITargetingService() {}

    virtual TargetingResult ClearAcquiredTarget(EntityId actor) = 0;
    virtual TargetingResult ClearPlayerTarget(EntityId actor) = 0;
    virtual TargetingResult ReleaseTarget(EntityId actor, EntityId expectedTarget,
                                          TargetReleaseMode mode) = 0;
    virtual TargetingResult ForceTarget(EntityId actor, EntityId target,
                                        std::uint32_t expiresAt) = 0;
};
}
}
