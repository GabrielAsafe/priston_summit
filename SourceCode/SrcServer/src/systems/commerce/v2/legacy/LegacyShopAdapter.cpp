#include "systems/commerce/v2/legacy/LegacyShopAdapter.h"

#include "gameplay/commerce/v2/ShopTransactionService.h"
#include "systems/commerce/v2/legacy/LegacyWalletAdapter.h"
#include "systems/inventory/v2/legacy/LegacyInventoryRuntime.h"
#include "systems/inventory/v2/legacy/LegacyInventoryReservationRuntime.h"
#include "systems/gameplay/inventory/v2/legacy/LegacyPotionStackAdapter.h"

namespace gameplay
{
namespace commerce
{
namespace legacy
{
ShopPurchaseResult LegacyShopAdapter::Purchase(const ShopPurchaseRequest& request)
{
    return transactionService_.Purchase(request);
}

ShopPotionPurchaseResult LegacyShopAdapter::PurchasePotion(const ShopPotionPurchaseRequest& request)
{
    return transactionService_.PurchasePotion(request);
}

ShopItemPurchaseResult LegacyShopAdapter::PurchaseItem(const ShopItemPurchaseRequest& request)
{
    return transactionService_.PurchaseItem(request);
}

ShopItemSaleResult LegacyShopAdapter::SellItem(const ShopItemSaleRequest& request)
{
    return transactionService_.SellItem(request);
}

ShopSaleResult LegacyShopAdapter::CreditSale(const ShopSaleRequest& request)
{
    return transactionService_.CreditSale(request);
}

ShopSaleResult LegacyShopAdapter::RollbackSale(const ShopSaleRequest& request)
{
    return transactionService_.RollbackSale(request);
}

LegacyShopAdapter& ServerLegacyShopAdapter()
{
    LegacyWalletAdapter& wallet = ServerLegacyWalletAdapter();
    inventory::legacy::LegacyInventoryRuntime& inventoryRuntime =
        inventory::legacy::ServerLegacyInventoryRuntime();
    inventory::legacy::LegacyPotionStackAdapter& potionStacks =
        inventory::legacy::ServerLegacyPotionStackAdapter();
    static ShopTransactionService transactionService(
        wallet, wallet, inventoryRuntime, inventoryRuntime,
        inventory::legacy::ServerInventoryReservationService(), potionStacks, potionStacks);
    static LegacyShopAdapter adapter(transactionService);
    return adapter;
}
}
}
}
