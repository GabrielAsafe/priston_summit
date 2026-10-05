#pragma once

#include <algorithm>

namespace gameplay
{
namespace combat
{
struct ActorDamageLifePlan
{
    int amount;
    int restoreAmount;
    bool consumeNonLethal;
    bool virtualLifeAbsorbed;

    ActorDamageLifePlan()
        : amount(0), restoreAmount(0), consumeNonLethal(false), virtualLifeAbsorbed(false) {}
};

class ActorDamageLifePolicy
{
public:
    static ActorDamageLifePlan Resolve(int currentLife,
                                       int maximumLife,
                                       int damage,
                                       bool virtualLifeProtected)
    {
        ActorDamageLifePlan plan;
        if(currentLife <= 0 || maximumLife <= 0 || damage <= 0)
            return plan;

        if(virtualLifeProtected && damage >= currentLife)
        {
            const int floorLife = std::max(1, maximumLife / 100);
            plan.virtualLifeAbsorbed = true;
            if(currentLife > floorLife)
            {
                plan.amount = currentLife - floorLife;
                plan.consumeNonLethal = true;
            }
            else if(currentLife < floorLife)
            {
                plan.restoreAmount = floorLife - currentLife;
            }
            return plan;
        }

        plan.amount = damage;
        return plan;
    }
};
}
}
