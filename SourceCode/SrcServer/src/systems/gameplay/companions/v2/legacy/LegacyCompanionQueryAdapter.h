#pragma once

#include "gameplay/companions/v2/CompanionQuery.h"

namespace gameplay
{
namespace companions
{
namespace legacy
{
class LegacyCompanionQueryAdapter final : public ICompanionQuery
{
public:
    std::uint32_t ActiveCount(PlayerId owner) const override;
    bool TryGetState(PlayerId owner, CompanionStateSnapshot& out) const override;
    std::uint32_t ConfiguredCrystalItemCode() const override;
};

const LegacyCompanionQueryAdapter& ServerLegacyCompanionQueryAdapter();
}
}
}
