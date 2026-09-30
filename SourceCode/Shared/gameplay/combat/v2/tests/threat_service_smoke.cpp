#include <cassert>

#include "gameplay/combat/v2/ThreatService.h"

namespace
{
class FakeThreatService final : public gameplay::combat::IThreatService
{
public:
    bool TryGetThreat(gameplay::EntityId actor, gameplay::combat::ThreatSnapshot& out) const override
    {
        if(!actor.Valid())
            return false;
        out.actor = actor;
        out.active = true;
        out.targetPlayer = gameplay::PlayerId(101u);
        out.targetEntity = gameplay::EntityId(202u);
        return true;
    }
};
}

int main()
{
    FakeThreatService service;
    gameplay::combat::ThreatSnapshot snapshot;
    assert(service.TryGetThreat(gameplay::EntityId(7u), snapshot));
    assert(snapshot.actor == gameplay::EntityId(7u));
    assert(snapshot.active);
    assert(snapshot.targetPlayer == gameplay::PlayerId(101u));
    assert(snapshot.targetEntity == gameplay::EntityId(202u));
    assert(snapshot.HasTarget());

    gameplay::combat::ThreatSnapshot empty;
    assert(!empty.HasTarget());
    assert(!service.TryGetThreat(gameplay::EntityId(), empty));
    return 0;
}
