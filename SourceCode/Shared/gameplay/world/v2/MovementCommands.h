#pragma once

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace world
{
enum class MovementCommandCode
{
    Ok = 0,
    InvalidRequest,
    SourceNotFound,
    TargetNotFound
};

struct MovementCommandResult
{
    MovementCommandCode code;
    bool moved;

    MovementCommandResult() : code(MovementCommandCode::InvalidRequest), moved(false) {}
    bool Ok() const { return code == MovementCommandCode::Ok && moved; }
};

class IMovementCommands
{
public:
    virtual ~IMovementCommands() {}
    virtual MovementCommandResult PushAway(EntityId target, EntityId source, int distance) = 0;
};
}
}
