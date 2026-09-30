#pragma once

#include "gameplay/commerce/v2/TradeService.h"
#include "gameplay/inventory/v2/Inventory.h"
#include "gameplay/inventory/v2/PotionStack.h"
#include "gameplay/commerce/v2/CoinWallet.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

namespace gameplay
{
namespace commerce
{
namespace legacy
{
class LegacyTradeAdapter final : public ITradeService
{
public:
    LegacyTradeAdapter(const player::legacy::LegacyOnlinePlayerRegistry& registry,
                       const IWalletQuery& walletQuery,
                       IWalletCommands& walletCommands,
                       inventory::IInventoryCommands& inventoryCommands,
                       const ICoinWalletQuery& coinQuery,
                       ICoinWalletCommands& coinCommands,
                       inventory::IPotionStackCommands& potionStackCommands);

    TradeIngressResult EvaluateIngress(const TradeIngressRequest& request) override;
    TradeCommitGuardResult CheckCommitReplay(const TradeCommitGuardRequest& request) override;
    void MarkCommitted(const TradeCommitGuardRequest& request) override;
    TradeGoldCommitResult CommitGold(const TradeGoldCommitRequest& request) override;
    TradeCoinCommitResult CommitCoins(const TradeCoinCommitRequest& request) override;
    TradeExactCommitResult CommitExactTrade(const TradeExactCommitRequest& request) override;
    inventory::InventoryTransferResult TransferItem(const TradeItemTransferRequest& request) override;
    inventory::InventoryMutationResult RemoveItem(PlayerId player, inventory::ItemInstanceId item) override;

private:
    const player::legacy::LegacyOnlinePlayerRegistry& registry_;
    const IWalletQuery& walletQuery_;
    IWalletCommands& walletCommands_;
    inventory::IInventoryCommands& inventoryCommands_;
    const ICoinWalletQuery& coinQuery_;
    ICoinWalletCommands& coinCommands_;
    inventory::IPotionStackCommands& potionStackCommands_;
};

LegacyTradeAdapter& ServerLegacyTradeAdapter();
}
}
}
