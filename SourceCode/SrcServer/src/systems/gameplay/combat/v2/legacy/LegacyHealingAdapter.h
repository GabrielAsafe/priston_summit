#pragma once

#include "gameplay/combat/v2/HealingService.h"

namespace gameplay
{
namespace combat
{
namespace legacy
{
class LegacyHealingAdapter final : public IHealingService
{
public:
    HealingResult Heal(EntityId source, EntityId target, int amount) override;
    HealingResult Heal(EntityId source, PlayerId target, int amount) override;
};

LegacyHealingAdapter& ServerLegacyHealingAdapter();
}
}
}
