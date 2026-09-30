#pragma once

#include "gameplay/commerce/v2/MixingService.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

namespace gameplay
{
namespace commerce
{
namespace legacy
{
class LegacyMixingAdapter final : public IMixingService
{
public:
    explicit LegacyMixingAdapter(const player::legacy::LegacyOnlinePlayerRegistry& registry);
    MixingResult Process(const MixingRequest& request) override;

private:
    const player::legacy::LegacyOnlinePlayerRegistry& registry_;
};

LegacyMixingAdapter& ServerLegacyMixingAdapter();
}
}
}
