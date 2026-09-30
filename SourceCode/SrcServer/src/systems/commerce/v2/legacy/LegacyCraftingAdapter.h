#pragma once

#include "gameplay/commerce/v2/CraftingService.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

namespace gameplay
{
namespace commerce
{
namespace legacy
{
class LegacyCraftingAdapter final : public ICraftingService
{
public:
    explicit LegacyCraftingAdapter(const player::legacy::LegacyOnlinePlayerRegistry& registry);

    CraftResult Craft(const CraftRequest& request) override;
    CraftingAuxiliaryResult ProcessAuxiliary(const CraftingAuxiliaryRequest& request) override;

private:
    const player::legacy::LegacyOnlinePlayerRegistry& registry_;
};

LegacyCraftingAdapter& ServerLegacyCraftingAdapter();
}
}
}
