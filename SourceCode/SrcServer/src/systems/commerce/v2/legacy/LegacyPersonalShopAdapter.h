#pragma once

#include "gameplay/commerce/v2/PersonalShopService.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"
#include "gameplay/commerce/v2/Wallet.h"
#include "gameplay/commerce/v2/CoinWallet.h"
#include "gameplay/inventory/v2/Inventory.h"
#include "gameplay/inventory/v2/PotionStack.h"

namespace gameplay
{
namespace commerce
{
namespace legacy
{
class LegacyPersonalShopAdapter final : public IPersonalShopService
{
public:
    LegacyPersonalShopAdapter(const player::legacy::LegacyOnlinePlayerRegistry& registry,
                              const IWalletQuery& walletQuery,
                              IWalletCommands& walletCommands,
                              const inventory::IInventoryQuery& inventoryQuery,
                              inventory::IInventoryCommands& inventoryCommands,
                              const ICoinWalletQuery& coinQuery,
                              ICoinWalletCommands& coinCommands,
                              const inventory::IPotionStackQuery& potionStackQuery,
                              inventory::IPotionStackCommands& potionStackCommands);

    PersonalShopIngressResult HandleIngress(const PersonalShopIngressRequest& request) override;
    PersonalShopPrepareResult PrepareTrade(const PersonalShopPrepareRequest& request) override;
    PersonalShopMessageResult ReadListingMessage(const PersonalShopMessageRequest& request) override;
    PersonalShopTradeResult CommitTrade(const PersonalShopTradeRequest& request) override;

private:
    const player::legacy::LegacyOnlinePlayerRegistry& registry_;
    const IWalletQuery& walletQuery_;
    IWalletCommands& walletCommands_;
    const inventory::IInventoryQuery& inventoryQuery_;
    inventory::IInventoryCommands& inventoryCommands_;
    const ICoinWalletQuery& coinQuery_;
    ICoinWalletCommands& coinCommands_;
    const inventory::IPotionStackQuery& potionStackQuery_;
    inventory::IPotionStackCommands& potionStackCommands_;
};

LegacyPersonalShopAdapter& ServerLegacyPersonalShopAdapter();
}
}
}
