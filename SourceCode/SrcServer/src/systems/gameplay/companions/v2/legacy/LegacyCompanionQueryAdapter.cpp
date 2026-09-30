#include "systems/gameplay/companions/v2/legacy/LegacyCompanionQueryAdapter.h"

#include "systems/gameplay/companions/v2/CompanionRuntimeQuery.h"

namespace gameplay
{
namespace companions
{
namespace legacy
{
std::uint32_t LegacyCompanionQueryAdapter::ActiveCount(PlayerId owner) const
{
    return runtime::ActiveCount(owner);
}

bool LegacyCompanionQueryAdapter::TryGetState(PlayerId owner, CompanionStateSnapshot& out) const
{
    return runtime::TryGetState(owner, out);
}

std::uint32_t LegacyCompanionQueryAdapter::ConfiguredCrystalItemCode() const
{
    return runtime::ConfiguredCrystalItemCode();
}

const LegacyCompanionQueryAdapter& ServerLegacyCompanionQueryAdapter()
{
    static LegacyCompanionQueryAdapter adapter;
    return adapter;
}
}
}
}
