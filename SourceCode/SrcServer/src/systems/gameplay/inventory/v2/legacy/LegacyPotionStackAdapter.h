#pragma once

#include "gameplay/inventory/v2/PotionStack.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

namespace gameplay
{
namespace inventory
{
namespace legacy
{
class LegacyPotionStackAdapter final : public IPotionStackQuery, public IPotionStackCommands
{
public:
    explicit LegacyPotionStackAdapter(const player::legacy::LegacyOnlinePlayerRegistry& registry);

    bool TryGetStack(PlayerId player, ItemId potion, PotionStackSnapshot& out) const override;
    PotionStackMutationResult AddStack(const PotionStackMutationRequest& request) override;
    PotionStackMutationResult RemoveStack(const PotionStackMutationRequest& request) override;
    PotionStackTransferResult TransferStack(const PotionStackTransferRequest& request) override;

private:
    const player::legacy::LegacyOnlinePlayerRegistry& registry_;
};

LegacyPotionStackAdapter& ServerLegacyPotionStackAdapter();
}
}
}
