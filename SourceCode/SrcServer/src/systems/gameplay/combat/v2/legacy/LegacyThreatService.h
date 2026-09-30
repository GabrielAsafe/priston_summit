#pragma once

#include "gameplay/combat/v2/ThreatService.h"

namespace gameplay
{
namespace combat
{
namespace legacy
{
class LegacyThreatService final : public IThreatService
{
public:
    bool TryGetThreat(EntityId actor, ThreatSnapshot& out) const override;
};

LegacyThreatService& ServerLegacyThreatService();
}
}
}
