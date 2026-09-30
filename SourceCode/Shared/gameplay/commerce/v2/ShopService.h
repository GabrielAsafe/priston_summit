#pragma once

#include "gameplay/commerce/v2/Wallet.h"
#include "gameplay/inventory/v2/Inventory.h"
#include "gameplay/inventory/v2/PotionStack.h"

namespace gameplay
{
namespace commerce
{
enum class ShopPurchaseKind
{
    Item,
    Potion
};

enum class ShopPotionPurchaseCode
{
    Purchased,
    InvalidPrice,
    InvalidQuantity,
    InvalidPotion,
    TotalOverflow,
    WalletNotFound,
    InsufficientFunds,
    PotionRejected,
    WalletRejected,
    RollbackFailed
};

struct ShopPotionPurchaseRequest
{
    PlayerId player;
    ItemId potion;
    CurrencyAmount unitPrice;
    inventory::PotionQuantity quantity;

    ShopPotionPurchaseRequest(PlayerId playerValue = PlayerId(),
                              ItemId potionValue = ItemId(),
                              CurrencyAmount unitPriceValue = 0,
                              inventory::PotionQuantity quantityValue = 0)
        : player(playerValue), potion(potionValue), unitPrice(unitPriceValue), quantity(quantityValue) {}
};

struct ShopPotionPurchaseResult
{
    ShopPotionPurchaseCode code;
    PlayerId player;
    ItemId potion;
    CurrencyAmount unitPrice;
    inventory::PotionQuantity quantity;
    CurrencyAmount total;
    CurrencyAmount before;
    CurrencyAmount after;

    ShopPotionPurchaseResult()
        : code(ShopPotionPurchaseCode::InvalidPotion), player(), potion(), unitPrice(0),
          quantity(0), total(0), before(0), after(0) {}

    bool Purchased() const { return code == ShopPotionPurchaseCode::Purchased; }
};

enum class ShopPurchaseCode
{
    Purchased,
    InvalidPrice,
    InvalidQuantity,
    TotalOverflow,
    WalletNotFound,
    InsufficientFunds,
    WalletRejected
};

struct ShopPurchaseRequest
{
    PlayerId player;
    CurrencyAmount unitPrice;
    int quantity;
    ShopPurchaseKind kind;

    ShopPurchaseRequest(PlayerId playerValue = PlayerId(),
                        CurrencyAmount unitPriceValue = 0,
                        int quantityValue = 1,
                        ShopPurchaseKind kindValue = ShopPurchaseKind::Item)
        : player(playerValue), unitPrice(unitPriceValue), quantity(quantityValue), kind(kindValue)
    {
    }
};

enum class ShopItemPurchaseCode
{
    Purchased,
    InvalidPrice,
    InvalidItem,
    WalletNotFound,
    InsufficientFunds,
    InventoryRejected,
    WalletRejected,
    RollbackFailed
};

struct ShopItemPurchaseRequest
{
    PlayerId player;
    inventory::ItemInstanceId item;
    CurrencyAmount price;

    ShopItemPurchaseRequest(PlayerId playerValue = PlayerId(),
                            inventory::ItemInstanceId itemValue = inventory::ItemInstanceId(),
                            CurrencyAmount priceValue = 0)
        : player(playerValue), item(itemValue), price(priceValue) {}
};

struct ShopItemPurchaseResult
{
    ShopItemPurchaseCode code;
    PlayerId player;
    inventory::ItemInstanceId item;
    CurrencyAmount price;
    CurrencyAmount before;
    CurrencyAmount after;

    ShopItemPurchaseResult()
        : code(ShopItemPurchaseCode::InvalidItem), player(), item(), price(0), before(0), after(0) {}

    bool Purchased() const { return code == ShopItemPurchaseCode::Purchased; }
};

struct ShopPurchaseResult
{
    ShopPurchaseCode code;
    PlayerId player;
    CurrencyAmount unitPrice;
    int quantity;
    CurrencyAmount total;
    CurrencyAmount before;
    CurrencyAmount after;

    ShopPurchaseResult()
        : code(ShopPurchaseCode::InvalidPrice), player(), unitPrice(0), quantity(0), total(0), before(0), after(0)
    {
    }

    bool Purchased() const { return code == ShopPurchaseCode::Purchased; }
};


enum class ShopItemSaleCode
{
    Sold,
    InvalidAmount,
    InvalidItem,
    WalletNotFound,
    CapacityExceeded,
    ItemNotFound,
    WalletRejected,
    RollbackFailed
};

struct ShopItemSaleRequest
{
    PlayerId player;
    inventory::ItemInstanceId item;
    CurrencyAmount amount;

    ShopItemSaleRequest(PlayerId playerValue = PlayerId(),
                        inventory::ItemInstanceId itemValue = inventory::ItemInstanceId(),
                        CurrencyAmount amountValue = 0)
        : player(playerValue), item(itemValue), amount(amountValue) {}
};

struct ShopItemSaleResult
{
    ShopItemSaleCode code;
    PlayerId player;
    inventory::ItemInstanceId item;
    CurrencyAmount amount;
    CurrencyAmount before;
    CurrencyAmount after;

    ShopItemSaleResult()
        : code(ShopItemSaleCode::InvalidItem), player(), item(), amount(0), before(0), after(0) {}

    bool Sold() const { return code == ShopItemSaleCode::Sold; }
};

enum class ShopSaleCode
{
    Credited,
    RolledBack,
    InvalidAmount,
    WalletNotFound,
    CapacityExceeded,
    WalletRejected,
    RollbackRejected
};

struct ShopSaleRequest
{
    PlayerId player;
    CurrencyAmount amount;

    ShopSaleRequest(PlayerId playerValue = PlayerId(), CurrencyAmount amountValue = 0)
        : player(playerValue), amount(amountValue)
    {
    }
};

struct ShopSaleResult
{
    ShopSaleCode code;
    PlayerId player;
    CurrencyAmount amount;
    CurrencyAmount before;
    CurrencyAmount after;

    ShopSaleResult()
        : code(ShopSaleCode::InvalidAmount), player(), amount(0), before(0), after(0)
    {
    }

    bool Credited() const { return code == ShopSaleCode::Credited; }
    bool RolledBack() const { return code == ShopSaleCode::RolledBack; }
};

class IShopService
{
public:
    virtual ~IShopService() {}
    virtual ShopPurchaseResult Purchase(const ShopPurchaseRequest& request) = 0;
    virtual ShopPotionPurchaseResult PurchasePotion(const ShopPotionPurchaseRequest& request) = 0;
    virtual ShopItemPurchaseResult PurchaseItem(const ShopItemPurchaseRequest& request) = 0;
    virtual ShopItemSaleResult SellItem(const ShopItemSaleRequest& request) = 0;
    virtual ShopSaleResult CreditSale(const ShopSaleRequest& request) = 0;
    virtual ShopSaleResult RollbackSale(const ShopSaleRequest& request) = 0;
};
}
}
