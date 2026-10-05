#include <cassert>
#include "gameplay/combat/v2/ActorDamageLifePolicy.h"

int main()
{
    using gameplay::combat::ActorDamageLifePolicy;

    {
        const auto plan = ActorDamageLifePolicy::Resolve(100, 100, 25, false);
        assert(plan.amount == 25);
        assert(!plan.consumeNonLethal);
        assert(!plan.virtualLifeAbsorbed);
    }
    {
        const auto plan = ActorDamageLifePolicy::Resolve(40, 100, 80, false);
        assert(plan.amount == 80);
        assert(!plan.consumeNonLethal);
        assert(!plan.virtualLifeAbsorbed);
    }
    {
        const auto plan = ActorDamageLifePolicy::Resolve(40, 1000, 80, true);
        assert(plan.amount == 30);
        assert(plan.consumeNonLethal);
        assert(plan.virtualLifeAbsorbed);
    }
    {
        const auto plan = ActorDamageLifePolicy::Resolve(5, 100, 99, true);
        assert(plan.amount == 4);
        assert(plan.consumeNonLethal);
        assert(plan.virtualLifeAbsorbed);
    }
    {
        const auto plan = ActorDamageLifePolicy::Resolve(1, 100, 99, true);
        assert(plan.amount == 0);
        assert(plan.restoreAmount == 0);
        assert(!plan.consumeNonLethal);
        assert(plan.virtualLifeAbsorbed);
    }
    {
        const auto plan = ActorDamageLifePolicy::Resolve(5, 1000, 99, true);
        assert(plan.amount == 0);
        assert(plan.restoreAmount == 5);
        assert(!plan.consumeNonLethal);
        assert(plan.virtualLifeAbsorbed);
    }
    {
        const auto plan = ActorDamageLifePolicy::Resolve(0, 100, 10, true);
        assert(plan.amount == 0);
        assert(!plan.virtualLifeAbsorbed);
    }
    return 0;
}
