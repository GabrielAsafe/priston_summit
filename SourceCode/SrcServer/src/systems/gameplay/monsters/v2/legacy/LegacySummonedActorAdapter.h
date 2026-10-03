#pragma once

#include "gameplay/monsters/v2/SummonedActorCommands.h"

namespace gameplay
{
namespace monsters
{
namespace legacy
{
class LegacySummonedActorAdapter final : public ISummonedActorCommands
{
public:
    SummonedActorConfigureResult Configure(const SummonedActorConfigureRequest& request) override;
};

LegacySummonedActorAdapter& ServerLegacySummonedActorAdapter();
}
}
}
