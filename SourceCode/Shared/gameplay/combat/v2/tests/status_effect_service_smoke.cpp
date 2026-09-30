#include "gameplay/combat/v2/StatusEffectService.h"

#include <cassert>

namespace
{
class FakeStatusEffectService final : public gameplay::combat::IStatusEffectService
{
public:
    gameplay::combat::StatusEffectResult ApplyStun(gameplay::EntityId target, int durationTicks) override { return Make(target.Valid() && durationTicks >= 0); }
    gameplay::combat::StatusEffectResult ApplyPoison(gameplay::EntityId target, int durationTicks, int damage) override { return Make(target.Valid() && durationTicks >= 0 && damage >= 0); }
    gameplay::combat::StatusEffectResult ApplyHolyMind(gameplay::EntityId target, int, int durationTicks) override { return Make(target.Valid() && durationTicks >= 0); }
    gameplay::combat::StatusEffectResult ApplyAssassinEye(gameplay::EntityId target, std::uint32_t expiresAt, int) override { return Make(target.Valid() && expiresAt != 0u); }
    gameplay::combat::StatusEffectResult ApplySlow(gameplay::EntityId target, const gameplay::combat::SlowEffect& effect) override { return Make(target.Valid() && effect.durationTicks >= 0); }
    gameplay::combat::StatusEffectResult ApplyElementalSlow16Hz(gameplay::EntityId target, int durationSeconds, int, int signedIceResistance) override { return Make(target.Valid() && durationSeconds >= 0 && signedIceResistance >= -100 && signedIceResistance <= 100); }
    gameplay::combat::StatusEffectResult ClearStatus(gameplay::EntityId target, gameplay::combat::StatusEffectKind) override { return Make(target.Valid()); }
    gameplay::combat::StatusEffectResult ApplyHeavenlyLight(gameplay::EntityId target, int, int durationTicks) override { return Make(target.Valid() && durationTicks >= 0); }
private:
    static gameplay::combat::StatusEffectResult Make(bool valid)
    {
        gameplay::combat::StatusEffectResult result;
        if(valid) { result.code = gameplay::combat::StatusEffectCode::Ok; result.applied = true; }
        return result;
    }
};
}

int main()
{
    FakeStatusEffectService service;
    assert(service.ApplyStun(gameplay::EntityId(1u), 160).Ok());
    assert(service.ApplyPoison(gameplay::EntityId(1u), 170, 25).Ok());
    assert(service.ApplySlow(gameplay::EntityId(1u), gameplay::combat::SlowEffect(136, 200, 0, true, true, true)).Ok());
    assert(service.ApplyElementalSlow16Hz(gameplay::EntityId(1u), 5, 180, -100).Ok());
    assert(service.ClearStatus(gameplay::EntityId(1u), gameplay::combat::StatusEffectKind::Stun).Ok());
    assert(!service.ApplyAssassinEye(gameplay::EntityId(), 100u, 5).Ok());
    return 0;
}
