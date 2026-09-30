#pragma once

#include "gameplay/world/v2/MovementCommands.h"

namespace gameplay
{
namespace world
{
namespace legacy
{
class LegacyMovementAdapter final : public IMovementCommands
{
public:
    MovementCommandResult PushAway(EntityId target, EntityId source, int distance) override;
};

LegacyMovementAdapter& ServerLegacyMovementAdapter();
}
}
}
