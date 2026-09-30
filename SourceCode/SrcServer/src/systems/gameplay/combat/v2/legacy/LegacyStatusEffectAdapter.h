#pragma once

#include "gameplay/combat/v2/StatusEffectService.h"

namespace gameplay
{
namespace combat
{
namespace legacy
{
class LegacyStatusEffectAdapter final : public IStatusEffectService
{
public:
    StatusEffectResult ApplyStun(EntityId target, int durationTicks) override;
    StatusEffectResult ApplyPoison(EntityId target, int durationTicks, int damage) override;
    StatusEffectResult ApplyHolyMind(EntityId target, int damageReduction, int durationTicks) override;
    StatusEffectResult ApplyAssassinEye(EntityId target, std::uint32_t expiresAt,
                                        int subCritical) override;
    StatusEffectResult ApplySlow(EntityId target, const SlowEffect& effect) override;
    StatusEffectResult ApplyElementalSlow16Hz(EntityId target, int durationSeconds,
                                               int moveSpeed, int signedIceResistance) override;
    StatusEffectResult ClearStatus(EntityId target, StatusEffectKind kind) override;
    StatusEffectResult ApplyHeavenlyLight(EntityId target, int attackRateReduction,
                                          int durationTicks) override;
};

LegacyStatusEffectAdapter& ServerLegacyStatusEffectAdapter();
}
}
}
