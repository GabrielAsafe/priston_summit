#include "gameplay/combat/v2/DamageContracts.h"

#include <cassert>

namespace
{
class FakeDamageService final : public gameplay::combat::IDamageService
{
public:
    gameplay::combat::DamageResult Apply(const gameplay::combat::DamageRequest& request) override
    {
        gameplay::combat::DamageResult result;
        result.rawDamage = request.rawDamage;
        if(!request.source.Valid() || !request.owner.Valid() || !request.target.Valid() || request.rawDamage <= 0)
        {
            result.rejectReason = gameplay::combat::DamageRejectReason::InvalidRequest;
            return result;
        }
        result.rejectReason = gameplay::combat::DamageRejectReason::None;
        result.finalDamage = request.rawDamage;
        result.remainingLife = 1;
        result.applied = true;
        return result;
    }
};
}

int main()
{
    FakeDamageService service;
    gameplay::combat::DamageRequest request(gameplay::EntityId(10u), gameplay::PlayerId(20u),
                                            gameplay::EntityId(30u), 40);
    const gameplay::combat::DamageResult accepted = service.Apply(request);
    assert(accepted.Accepted());
    assert(accepted.finalDamage == 40);

    const gameplay::combat::DamageResult rejected = service.Apply(gameplay::combat::DamageRequest());
    assert(!rejected.Accepted());
    assert(rejected.rejectReason == gameplay::combat::DamageRejectReason::InvalidRequest);
    return 0;
}
