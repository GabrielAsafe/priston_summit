#include "gameplay/combat/v2/TargetingService.h"

#include <cassert>

namespace
{
class FakeTargetingService final : public gameplay::combat::ITargetingService
{
public:
    gameplay::combat::TargetingResult ClearAcquiredTarget(gameplay::EntityId actor) override
    {
        return Make(actor.Valid());
    }

    gameplay::combat::TargetingResult ClearPlayerTarget(gameplay::EntityId actor) override
    {
        return Make(actor.Valid());
    }

    gameplay::combat::TargetingResult ReleaseTarget(gameplay::EntityId actor,
        gameplay::EntityId target, gameplay::combat::TargetReleaseMode) override
    {
        return Make(actor.Valid() && target.Valid());
    }

    gameplay::combat::TargetingResult ForceTarget(gameplay::EntityId actor,
        gameplay::EntityId target, std::uint32_t expiresAt) override
    {
        return Make(actor.Valid() && target.Valid() && expiresAt != 0u);
    }

private:
    static gameplay::combat::TargetingResult Make(bool valid)
    {
        gameplay::combat::TargetingResult result;
        if(valid)
        {
            result.code = gameplay::combat::TargetingCode::Ok;
            result.changed = true;
        }
        return result;
    }
};
}

int main()
{
    FakeTargetingService service;
    assert(service.ClearAcquiredTarget(gameplay::EntityId(1u)).Ok());
    assert(service.ClearPlayerTarget(gameplay::EntityId(1u)).Ok());
    assert(service.ReleaseTarget(gameplay::EntityId(1u), gameplay::EntityId(2u),
        gameplay::combat::TargetReleaseMode::AcquiredAndForced).changed);
    assert(service.ForceTarget(gameplay::EntityId(1u), gameplay::EntityId(2u), 1000u).Ok());
    assert(!service.ForceTarget(gameplay::EntityId(), gameplay::EntityId(2u), 1000u).Ok());
    return 0;
}
