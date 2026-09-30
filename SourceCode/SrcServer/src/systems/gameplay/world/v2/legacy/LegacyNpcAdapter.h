#pragma once

#include "gameplay/world/v2/INpcQuery.h"

namespace gameplay
{
namespace world
{
namespace legacy
{
class LegacyNpcAdapter final : public INpcQuery
{
public:
    bool TryGetNpc(EntityId entity, NpcSnapshot& out) const override;
};

const LegacyNpcAdapter& ServerLegacyNpcAdapter();
}
}
}
