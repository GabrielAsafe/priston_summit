#pragma once

#include "systems/gameplay/combat/v2/legacy/LegacyCombatBuffAdapter.h"
#include "systems/gameplay/character/core/character.h"

namespace gameplay
{
namespace combat
{
namespace legacy
{
class ScopedAttackRatingBuff
{
public:
    ScopedAttackRatingBuff(smCHAR* actor, int delta)
        : target_(), delta_(), active_(false)
    {
        if(!actor || delta == 0)
            return;
        target_ = EntityId(static_cast<std::uint32_t>(actor->dwObjectSerial));
        delta_.attackRating = delta;
        active_ = ServerLegacyCombatBuffAdapter().Apply(target_, delta_).Ok();
    }

    ~ScopedAttackRatingBuff()
    {
        Release();
    }

    void Release()
    {
        if(!active_)
            return;
        (void)ServerLegacyCombatBuffAdapter().Remove(target_, delta_);
        active_ = false;
    }

    bool Active() const { return active_; }

private:
    ScopedAttackRatingBuff(const ScopedAttackRatingBuff&);
    ScopedAttackRatingBuff& operator=(const ScopedAttackRatingBuff&);

    EntityId target_;
    CombatBuffDelta delta_;
    bool active_;
};
}
}
}
