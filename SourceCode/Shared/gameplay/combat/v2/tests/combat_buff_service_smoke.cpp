#include "gameplay/combat/v2/CombatBuffService.h"

#include <cassert>

namespace
{
class FakeCombatBuffService final : public gameplay::combat::ICombatBuffService
{
public:
    gameplay::combat::CombatBuffResult Apply(gameplay::EntityId target,
                                              const gameplay::combat::CombatBuffDelta&) override
    {
        return Make(target.Valid());
    }

    gameplay::combat::CombatBuffResult Remove(gameplay::EntityId target,
                                               const gameplay::combat::CombatBuffDelta&) override
    {
        return Make(target.Valid());
    }

private:
    static gameplay::combat::CombatBuffResult Make(bool valid)
    {
        gameplay::combat::CombatBuffResult result;
        if(valid)
        {
            result.code = gameplay::combat::CombatBuffCode::Ok;
            result.applied = true;
        }
        return result;
    }
};
}

int main()
{
    gameplay::combat::CombatBuffDelta delta;
    delta.attackMin = 10;
    delta.absorption = -5;
    delta.resistance[2] = 7;

    FakeCombatBuffService service;
    assert(service.Apply(gameplay::EntityId(1u), delta).Ok());
    assert(service.Remove(gameplay::EntityId(1u), delta).Ok());
    assert(!service.Apply(gameplay::EntityId(), delta).Ok());
    return 0;
}
