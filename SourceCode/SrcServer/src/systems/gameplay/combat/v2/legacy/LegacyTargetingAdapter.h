#pragma once

#include "gameplay/combat/v2/TargetingService.h"

namespace gameplay
{
namespace combat
{
namespace legacy
{
class LegacyTargetingAdapter final : public ITargetingService
{
public:
    TargetingResult ClearAcquiredTarget(EntityId actor) override;
    TargetingResult ReleaseTarget(EntityId actor, EntityId expectedTarget,
                                  TargetReleaseMode mode) override;
    TargetingResult ForceTarget(EntityId actor, EntityId target,
                                std::uint32_t expiresAt) override;
};

LegacyTargetingAdapter& ServerLegacyTargetingAdapter();
}
}
}
