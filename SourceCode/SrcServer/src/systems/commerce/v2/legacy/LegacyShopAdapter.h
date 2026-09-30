#pragma once

#include "gameplay/commerce/v2/ShopService.h"

namespace gameplay
{
namespace commerce
{
namespace legacy
{
class LegacyShopAdapter final : public IShopService
{
public:
    LegacyShopAdapter(const IWalletQuery& walletQuery,
                      IWalletCommands& walletCommands,
                      const inventory::IInventoryQuery& inventoryQuery,
                      inventory::IInventoryCommands& inventoryCommands,
                      const inventory::IPotionStackQuery& potionStackQuery,
                      inventory::IPotionStackCommands& potionStackCommands);

    ShopPurchaseResult Purchase(const ShopPurchaseRequest& request) override;
    ShopPotionPurchaseResult PurchasePotion(const ShopPotionPurchaseRequest& request) override;
    ShopItemPurchaseResult PurchaseItem(const ShopItemPurchaseRequest& request) override;
    ShopItemSaleResult SellItem(const ShopItemSaleRequest& request) override;
    ShopSaleResult CreditSale(const ShopSaleRequest& request) override;
    ShopSaleResult RollbackSale(const ShopSaleRequest& request) override;

private:
    const IWalletQuery& walletQuery_;
    IWalletCommands& walletCommands_;
    const inventory::IInventoryQuery& inventoryQuery_;
    inventory::IInventoryCommands& inventoryCommands_;
    const inventory::IPotionStackQuery& potionStackQuery_;
    inventory::IPotionStackCommands& potionStackCommands_;
};

LegacyShopAdapter& ServerLegacyShopAdapter();
}
}
}
