#pragma once

#include "gameplay/commerce/v2/AgingService.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

namespace gameplay
{
namespace commerce
{
namespace legacy
{
class LegacyAgingAdapter final : public IAgingService
{
public:
    explicit LegacyAgingAdapter(const player::legacy::LegacyOnlinePlayerRegistry& registry);
    AgingResult Process(const AgingRequest& request) override;

private:
    const player::legacy::LegacyOnlinePlayerRegistry& registry_;
};

LegacyAgingAdapter& ServerLegacyAgingAdapter();
}
}
}
