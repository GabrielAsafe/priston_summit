#pragma once

#include "gameplay/commerce/v2/PersonalShopService.h"
#include "gameplay/commerce/v2/PersonalShopSession.h"
#include "gameplay/commerce/v2/PersonalShopTransaction.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"
#include "gameplay/commerce/v2/Wallet.h"
#include "gameplay/commerce/v2/CoinWallet.h"
#include "gameplay/inventory/v2/Inventory.h"
#include "gameplay/inventory/v2/InventoryReservation.h"
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
                              const inventory::IInventoryQuery& inventoryQuery,
                              IPersonalShopSessionService& sessionService,
                              const ICoinWalletQuery& coinQuery,
                              const inventory::IPotionStackQuery& potionStackQuery,
                              IPersonalShopTransactionService& transactionService);

    PersonalShopIngressResult HandleIngress(const PersonalShopIngressRequest& request) override;
    PersonalShopPrepareResult PrepareTrade(const PersonalShopPrepareRequest& request) override;
    PersonalShopMessageResult ReadListingMessage(const PersonalShopMessageRequest& request) override;
    PersonalShopTradeResult CommitTrade(const PersonalShopTradeRequest& request) override;

private:
    const player::legacy::LegacyOnlinePlayerRegistry& registry_;
    const IWalletQuery& walletQuery_;
    const inventory::IInventoryQuery& inventoryQuery_;
    IPersonalShopSessionService& sessionService_;
    const ICoinWalletQuery& coinQuery_;
    const inventory::IPotionStackQuery& potionStackQuery_;
    IPersonalShopTransactionService& transactionService_;
};

LegacyPersonalShopAdapter& ServerLegacyPersonalShopAdapter();
}
}
}
