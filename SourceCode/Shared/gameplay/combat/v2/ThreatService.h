#pragma once

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace combat
{
struct ThreatSnapshot
{
    EntityId actor;
    PlayerId targetPlayer;
    EntityId targetEntity;
    bool active;

    ThreatSnapshot()
        : actor(), targetPlayer(), targetEntity(), active(false) {}

    bool HasTarget() const
    {
        return targetPlayer.Valid() || targetEntity.Valid();
    }
};

class IThreatService
{
public:
    virtual ~IThreatService() {}
    virtual bool TryGetThreat(EntityId actor, ThreatSnapshot& out) const = 0;
};
}
}
