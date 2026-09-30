#include "gameplay/commerce/v2/ShopService.h"

#include <cassert>
#include <iostream>

namespace
{
class FakeShopService final : public gameplay::commerce::IShopService
{
public:
    gameplay::commerce::ShopPurchaseResult Purchase(
        const gameplay::commerce::ShopPurchaseRequest& request) override
    {
        gameplay::commerce::ShopPurchaseResult result;
        result.player = request.player;
        result.unitPrice = request.unitPrice;
        result.quantity = request.quantity;
        result.total = request.unitPrice * request.quantity;
        result.before = 1000;
        result.after = result.before - result.total;
        result.code = result.total <= result.before
                          ? gameplay::commerce::ShopPurchaseCode::Purchased
                          : gameplay::commerce::ShopPurchaseCode::InsufficientFunds;
        return result;
    }

    gameplay::commerce::ShopPotionPurchaseResult PurchasePotion(
        const gameplay::commerce::ShopPotionPurchaseRequest& request) override
    {
        gameplay::commerce::ShopPotionPurchaseResult result;
        result.player = request.player; result.potion = request.potion; result.unitPrice = request.unitPrice;
        result.quantity = request.quantity; result.total = request.unitPrice * request.quantity;
        result.before = 1000; result.after = result.before - result.total;
        result.code = request.potion.Valid() && result.total <= result.before
                          ? gameplay::commerce::ShopPotionPurchaseCode::Purchased
                          : gameplay::commerce::ShopPotionPurchaseCode::PotionRejected;
        return result;
    }

    gameplay::commerce::ShopItemPurchaseResult PurchaseItem(
        const gameplay::commerce::ShopItemPurchaseRequest& request) override
    {
        gameplay::commerce::ShopItemPurchaseResult result;
        result.player = request.player;
        result.item = request.item;
        result.price = request.price;
        result.before = 1000;
        result.after = result.before - request.price;
        result.code = request.item.Valid() && request.price <= result.before
                          ? gameplay::commerce::ShopItemPurchaseCode::Purchased
                          : gameplay::commerce::ShopItemPurchaseCode::InventoryRejected;
        return result;
    }

    gameplay::commerce::ShopItemSaleResult SellItem(
        const gameplay::commerce::ShopItemSaleRequest& request) override
    {
        gameplay::commerce::ShopItemSaleResult result;
        result.player = request.player;
        result.item = request.item;
        result.amount = request.amount;
        result.before = 500;
        result.after = 500 + request.amount;
        result.code = request.item.Valid()
                          ? gameplay::commerce::ShopItemSaleCode::Sold
                          : gameplay::commerce::ShopItemSaleCode::InvalidItem;
        return result;
    }

    gameplay::commerce::ShopSaleResult CreditSale(
        const gameplay::commerce::ShopSaleRequest& request) override
    {
        gameplay::commerce::ShopSaleResult result;
        result.player = request.player;
        result.amount = request.amount;
        result.before = 500;
        result.after = 500 + request.amount;
        result.code = request.amount >= 0
                          ? gameplay::commerce::ShopSaleCode::Credited
                          : gameplay::commerce::ShopSaleCode::InvalidAmount;
        return result;
    }

    gameplay::commerce::ShopSaleResult RollbackSale(
        const gameplay::commerce::ShopSaleRequest& request) override
    {
        gameplay::commerce::ShopSaleResult result;
        result.player = request.player;
        result.amount = request.amount;
        result.before = 500 + request.amount;
        result.after = 500;
        result.code = gameplay::commerce::ShopSaleCode::RolledBack;
        return result;
    }
};
}

int main()
{
    using namespace gameplay;
    using namespace gameplay::commerce;

    FakeShopService service;
    ShopPurchaseResult item = service.Purchase(
        ShopPurchaseRequest(PlayerId(1u), 250, 1, ShopPurchaseKind::Item));
    assert(item.Purchased());
    assert(item.total == 250);
    assert(item.after == 750);

    const inventory::ItemInstanceId itemId(ItemId(0x1234u), 44u, 55u);
    ShopItemPurchaseResult exact = service.PurchaseItem(
        ShopItemPurchaseRequest(PlayerId(1u), itemId, 250));
    assert(exact.Purchased());
    assert(exact.item == itemId);
    assert(exact.after == 750);

    ShopPotionPurchaseResult potion = service.PurchasePotion(
        ShopPotionPurchaseRequest(PlayerId(1u), ItemId(0x200u), 30, 10));
    assert(potion.Purchased());
    assert(potion.total == 300);

    ShopItemSaleResult exactSale = service.SellItem(
        ShopItemSaleRequest(PlayerId(1u), itemId, 125));
    assert(exactSale.Sold());
    assert(exactSale.after == 625);

    ShopSaleResult sale = service.CreditSale(ShopSaleRequest(PlayerId(1u), 125));
    assert(sale.Credited());
    assert(sale.before == 500);
    assert(sale.after == 625);

    ShopSaleResult rollback = service.RollbackSale(ShopSaleRequest(PlayerId(1u), 125));
    assert(rollback.RolledBack());
    assert(rollback.after == 500);

    std::cout << "shop service smoke: PASS\n";
    return 0;
}
