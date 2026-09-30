#pragma once

#include "gameplay/inventory/v2/Inventory.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

namespace gameplay
{
namespace inventory
{
namespace legacy
{
class LegacyInventoryRuntime final : public IInventoryQuery, public IInventoryCommands
{
public:
    explicit LegacyInventoryRuntime(const player::legacy::LegacyOnlinePlayerRegistry& registry);

    InventoryPresenceResult Contains(PlayerId player, ItemInstanceId item) const override;
    bool TryGetSnapshot(PlayerId player, InventorySnapshot& out) const override;
    InventoryMutationResult Add(PlayerId player, ItemInstanceId item) override;
    InventoryMutationResult Remove(PlayerId player, ItemInstanceId item) override;
    InventoryTransferResult Transfer(PlayerId source, PlayerId destination, ItemInstanceId item) override;
    InventoryGrantResult Grant(const ItemGrant& grant) override;

private:
    const player::legacy::LegacyOnlinePlayerRegistry& registry_;
};

LegacyInventoryRuntime& ServerLegacyInventoryRuntime();
}
}
}
