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
    explicit LegacyShopAdapter(IShopService& transactionService)
        : transactionService_(transactionService) {}

    ShopPurchaseResult Purchase(const ShopPurchaseRequest& request) override;
    ShopPotionPurchaseResult PurchasePotion(const ShopPotionPurchaseRequest& request) override;
    ShopItemPurchaseResult PurchaseItem(const ShopItemPurchaseRequest& request) override;
    ShopItemSaleResult SellItem(const ShopItemSaleRequest& request) override;
    ShopSaleResult CreditSale(const ShopSaleRequest& request) override;
    ShopSaleResult RollbackSale(const ShopSaleRequest& request) override;

private:
    IShopService& transactionService_;
};

LegacyShopAdapter& ServerLegacyShopAdapter();
}
}
}
