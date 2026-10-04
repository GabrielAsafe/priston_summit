#pragma once

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace commerce
{
struct VipEntitlementSnapshot
{
    PlayerId player;
    bool active;
    int tier;

    VipEntitlementSnapshot() : player(), active(false), tier(0) {}
};

enum class VipGrantCode
{
    Granted = 0,
    InvalidPlayer,
    InvalidTier,
    InvalidDuration,
    PersistenceRejected
};

struct VipGrantRequest
{
    PlayerId player;
    int tier;
    int days;

    VipGrantRequest(PlayerId playerValue = PlayerId(), int tierValue = 0, int daysValue = 0)
        : player(playerValue), tier(tierValue), days(daysValue) {}
};

class IPremiumEntitlementRepository
{
public:
    virtual ~IPremiumEntitlementRepository() {}
    virtual bool TryGetVip(PlayerId player, VipEntitlementSnapshot& out) = 0;
    virtual bool GrantVip(PlayerId player, int tier, int days) = 0;
};

class PremiumEntitlementService final
{
public:
    explicit PremiumEntitlementService(IPremiumEntitlementRepository& repository)
        : repository_(repository) {}

    bool TryGetVip(PlayerId player, VipEntitlementSnapshot& out)
    {
        if(!player.Valid())
            return false;
        if(!repository_.TryGetVip(player, out))
            return false;
        // Repository may report a stale tier for an inactive row.  GREEN owns
        // the invariant that inactive entitlements expose no effective tier.
        if(!out.active)
            out.tier = 0;
        return true;
    }

    VipGrantCode GrantVip(const VipGrantRequest& request)
    {
        if(!request.player.Valid())
            return VipGrantCode::InvalidPlayer;
        if(request.tier <= 0)
            return VipGrantCode::InvalidTier;
        if(request.days <= 0)
            return VipGrantCode::InvalidDuration;
        return repository_.GrantVip(request.player, request.tier, request.days)
                   ? VipGrantCode::Granted
                   : VipGrantCode::PersistenceRejected;
    }

private:
    IPremiumEntitlementRepository& repository_;
};
}
}
