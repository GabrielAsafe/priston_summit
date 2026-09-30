#include "gameplay/combat/v2/HealingService.h"

#include <cassert>

namespace
{
class FakeHealingService final : public gameplay::combat::IHealingService
{
public:
    gameplay::combat::HealingResult Heal(gameplay::EntityId source, gameplay::EntityId target, int amount) override
    {
        return Build(source.Valid(), target.Valid(), amount, 40, 100);
    }

    gameplay::combat::HealingResult Heal(gameplay::EntityId source, gameplay::PlayerId target, int amount) override
    {
        return Build(source.Valid(), target.Valid(), amount, 80, 100);
    }

private:
    static gameplay::combat::HealingResult Build(bool sourceValid, bool targetValid, int amount,
                                                  int before, int maximum)
    {
        gameplay::combat::HealingResult result;
        result.requested = amount;
        result.before = before;
        result.after = before;
        result.maximum = maximum;
        if(!sourceValid || !targetValid || amount <= 0)
            return result;
        result.code = gameplay::combat::HealingCode::Ok;
        result.delivered = true;
        result.after = before + amount;
        if(result.after > maximum)
            result.after = maximum;
        result.applied = result.after > result.before;
        return result;
    }
};
}

int main()
{
    FakeHealingService healing;

    const gameplay::combat::HealingResult entity = healing.Heal(
        gameplay::EntityId(1u), gameplay::EntityId(2u), 75);
    assert(entity.Accepted());
    assert(entity.requested == 75);
    assert(entity.before == 40);
    assert(entity.after == 100);
    assert(entity.applied);

    const gameplay::combat::HealingResult player = healing.Heal(
        gameplay::EntityId(1u), gameplay::PlayerId(3u), 15);
    assert(player.Accepted());
    assert(player.after == 95);

    const gameplay::combat::HealingResult rejected = healing.Heal(
        gameplay::EntityId(), gameplay::EntityId(2u), 10);
    assert(!rejected.Accepted());
    assert(rejected.code == gameplay::combat::HealingCode::InvalidRequest);
    return 0;
}
