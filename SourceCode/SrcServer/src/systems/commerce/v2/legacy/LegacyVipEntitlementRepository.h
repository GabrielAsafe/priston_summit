#pragma once

#include "gameplay/commerce/v2/PremiumEntitlementService.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

namespace gameplay
{
namespace commerce
{
namespace legacy
{
class LegacyVipEntitlementRepository final : public IPremiumEntitlementRepository
{
public:
    explicit LegacyVipEntitlementRepository(const player::legacy::LegacyOnlinePlayerRegistry& players)
        : players_(players) {}

    bool TryGetVip(PlayerId player, VipEntitlementSnapshot& out) override;
    bool GrantVip(PlayerId player, int tier, int days) override;

private:
    const player::legacy::LegacyOnlinePlayerRegistry& players_;
};

PremiumEntitlementService& ServerPremiumEntitlementService();
}
}
}
