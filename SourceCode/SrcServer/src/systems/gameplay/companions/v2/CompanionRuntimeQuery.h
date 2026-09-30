#pragma once

#include <cstdint>

#include "gameplay/companions/v2/CompanionQuery.h"

namespace gameplay
{
namespace companions
{
namespace runtime
{
std::uint32_t ActiveCount(PlayerId owner);
bool TryGetState(PlayerId owner, CompanionStateSnapshot& out);
std::uint32_t ConfiguredCrystalItemCode();
}
}
}
