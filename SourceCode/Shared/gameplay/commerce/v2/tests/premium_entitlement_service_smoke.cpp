#include "gameplay/commerce/v2/PremiumEntitlementService.h"

#include <cassert>

namespace
{
class Repo final : public gameplay::commerce::IPremiumEntitlementRepository
{
public:
    bool TryGetVip(gameplay::PlayerId player, gameplay::commerce::VipEntitlementSnapshot& out) override
    {
        ++reads;
        out.player = player;
        out.active = active;
        out.tier = tier;
        return readable;
    }
    bool GrantVip(gameplay::PlayerId player, int tierValue, int daysValue) override
    {
        ++grants; lastPlayer=player; tier=tierValue; days=daysValue; active=true; return grantable;
    }
    int reads=0, grants=0, tier=3, days=0; bool active=true, readable=true, grantable=true;
    gameplay::PlayerId lastPlayer;
};
}

int main()
{
    Repo repo;
    gameplay::commerce::PremiumEntitlementService service(repo);
    gameplay::commerce::VipEntitlementSnapshot snapshot;
    assert(service.TryGetVip(gameplay::PlayerId(10), snapshot));
    assert(snapshot.active && snapshot.tier == 3);
    repo.active=false;
    assert(service.TryGetVip(gameplay::PlayerId(10), snapshot));
    assert(!snapshot.active && snapshot.tier == 0);
    assert(service.GrantVip(gameplay::commerce::VipGrantRequest(gameplay::PlayerId(10), 2, 30)) == gameplay::commerce::VipGrantCode::Granted);
    assert(repo.grants == 1 && repo.tier == 2 && repo.days == 30);
    assert(service.GrantVip(gameplay::commerce::VipGrantRequest(gameplay::PlayerId(), 2, 30)) == gameplay::commerce::VipGrantCode::InvalidPlayer);
    assert(service.GrantVip(gameplay::commerce::VipGrantRequest(gameplay::PlayerId(10), 0, 30)) == gameplay::commerce::VipGrantCode::InvalidTier);
    return 0;
}
