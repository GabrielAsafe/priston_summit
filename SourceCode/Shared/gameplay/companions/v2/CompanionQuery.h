#pragma once

#include <cstdint>
#include <string>

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace companions
{
inline constexpr std::uint32_t COMPANION_MAX_ACTIVE = 4u;

enum class CompanionBehaviorMode : std::uint8_t
{
    Aggressive = 0u,
    Follow = 1u,
    Passive = 2u
};

struct CompanionStateSnapshot
{
    CompanionStateSnapshot()
        : activeCount(0u), mode(CompanionBehaviorMode::Aggressive) {}

    std::uint32_t activeCount;
    CompanionBehaviorMode mode;
    std::string selectedName;
};

class ICompanionQuery
{
public:
    virtual ~ICompanionQuery() {}
    virtual std::uint32_t ActiveCount(PlayerId owner) const = 0;
    virtual bool TryGetState(PlayerId owner, CompanionStateSnapshot& out) const = 0;
    virtual std::uint32_t ConfiguredCrystalItemCode() const = 0;
};
}
}
