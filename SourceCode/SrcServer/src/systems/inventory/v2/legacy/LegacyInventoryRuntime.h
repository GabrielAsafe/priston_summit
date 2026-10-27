#pragma once

#include "gameplay/inventory/v2/Inventory.h"
#include "gameplay/companions/v2/CompanionTradeOnlineIdentitySnapshot.h"
#include "gameplay/companions/v2/CompanionTradeOnlineDatReconciliation.h"
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
    // Read-only, generation-bound online inventory ledger for preparing a trade.
    // The online ledger cannot provide complete item payloads: never commit from it.
    bool TryGetTradeIdentitySnapshot(PlayerHandle session,
        companions::OnlineTradeIdentitySnapshot& out) const;
    bool MatchesTradeIdentitySnapshot(
        const companions::OnlineTradeIdentitySnapshot& expected) const;
    // Read-only best-effort reconciliation. Even MatchReadOnly MUST NOT
    // authorize a transfer from the saved DAT or mutate the online inventory.
    companions::v2::OnlineTradePersistedCheck CheckTradePersistedIdentity(
        PlayerHandle session) const;

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
