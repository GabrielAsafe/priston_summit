#pragma once

#include <limits>

#include "gameplay/commerce/v2/ShopService.h"
#include "gameplay/inventory/v2/InventoryReservation.h"

namespace gameplay
{
namespace commerce
{
class ShopTransactionService final : public IShopService
{
public:
    ShopTransactionService(const IWalletQuery& walletQuery,
                           IWalletCommands& walletCommands,
                           const inventory::IInventoryQuery& inventoryQuery,
                           inventory::IInventoryCommands& inventoryCommands,
                           inventory::IInventoryReservationService& inventoryReservations,
                           const inventory::IPotionStackQuery& potionStackQuery,
                           inventory::IPotionStackCommands& potionStackCommands)
        : walletQuery_(walletQuery), walletCommands_(walletCommands), inventoryQuery_(inventoryQuery),
          inventoryCommands_(inventoryCommands), inventoryReservations_(inventoryReservations),
          potionStackQuery_(potionStackQuery), potionStackCommands_(potionStackCommands)
    {
    }

    ShopPurchaseResult Purchase(const ShopPurchaseRequest& request) override
    {
        ShopPurchaseResult result;
        result.player = request.player;
        result.unitPrice = request.unitPrice;
        result.quantity = request.quantity;
        if(request.unitPrice < 0) { result.code = ShopPurchaseCode::InvalidPrice; return result; }
        if(request.quantity <= 0) { result.code = ShopPurchaseCode::InvalidQuantity; return result; }
        if(request.unitPrice > 0 && static_cast<CurrencyAmount>(request.quantity) >
           std::numeric_limits<CurrencyAmount>::max() / request.unitPrice)
        { result.code = ShopPurchaseCode::TotalOverflow; return result; }
        result.total = request.unitPrice * static_cast<CurrencyAmount>(request.quantity);

        WalletSnapshot wallet;
        if(!walletQuery_.TryGetWallet(request.player, wallet))
        { result.code = ShopPurchaseCode::WalletNotFound; return result; }
        result.before = wallet.gold; result.after = wallet.gold;
        if(result.total == 0) { result.code = ShopPurchaseCode::Purchased; return result; }
        if(wallet.gold < result.total) { result.code = ShopPurchaseCode::InsufficientFunds; return result; }

        const WalletReason reason = request.kind == ShopPurchaseKind::Potion
                                        ? WalletReason::NpcBuyPotion : WalletReason::NpcBuyItem;
        const WalletMutationResult debit = walletCommands_.Debit(request.player, result.total, reason);
        result.after = debit.after;
        result.code = debit.Ok() ? ShopPurchaseCode::Purchased : ShopPurchaseCode::WalletRejected;
        return result;
    }

    ShopPotionPurchaseResult PurchasePotion(const ShopPotionPurchaseRequest& request) override
    {
        ShopPotionPurchaseResult result;
        result.player=request.player; result.potion=request.potion; result.unitPrice=request.unitPrice; result.quantity=request.quantity;
        if(request.unitPrice < 0) { result.code=ShopPotionPurchaseCode::InvalidPrice; return result; }
        if(request.quantity <= 0) { result.code=ShopPotionPurchaseCode::InvalidQuantity; return result; }
        if(!request.potion.Valid()) { result.code=ShopPotionPurchaseCode::InvalidPotion; return result; }
        if(request.unitPrice > 0 && request.quantity > std::numeric_limits<CurrencyAmount>::max()/request.unitPrice)
        { result.code=ShopPotionPurchaseCode::TotalOverflow; return result; }
        result.total=request.unitPrice*request.quantity;
        WalletSnapshot wallet;
        if(!walletQuery_.TryGetWallet(request.player,wallet)) { result.code=ShopPotionPurchaseCode::WalletNotFound; return result; }
        result.before=wallet.gold; result.after=wallet.gold;
        if(wallet.gold<result.total) { result.code=ShopPotionPurchaseCode::InsufficientFunds; return result; }
        inventory::PotionStackSnapshot stack;
        if(!potionStackQuery_.TryGetStack(request.player,request.potion,stack) || stack.quantity<0 ||
           request.quantity > static_cast<inventory::PotionQuantity>(std::numeric_limits<int>::max())-stack.quantity)
        { result.code=ShopPotionPurchaseCode::PotionRejected; return result; }
        const inventory::PotionStackMutationResult grant=potionStackCommands_.AddStack(
            inventory::PotionStackMutationRequest(request.player,request.potion,request.quantity));
        if(!grant.Applied())
        { result.code=grant.code==inventory::PotionStackMutationCode::RollbackFailed?ShopPotionPurchaseCode::RollbackFailed:ShopPotionPurchaseCode::PotionRejected; return result; }
        if(result.total==0) { result.code=ShopPotionPurchaseCode::Purchased; return result; }
        const WalletMutationResult debit=walletCommands_.Debit(request.player,result.total,WalletReason::NpcBuyPotion);
        result.after=debit.after;
        if(!debit.Ok())
        {
            const inventory::PotionStackMutationResult rollback=potionStackCommands_.RemoveStack(
                inventory::PotionStackMutationRequest(request.player,request.potion,request.quantity));
            result.code=rollback.Applied()?ShopPotionPurchaseCode::WalletRejected:ShopPotionPurchaseCode::RollbackFailed;
            return result;
        }
        result.code=ShopPotionPurchaseCode::Purchased;
        return result;
    }

    ShopItemPurchaseResult PurchaseItem(const ShopItemPurchaseRequest& request) override
    {
        ShopItemPurchaseResult result;
        result.player=request.player; result.item=request.item; result.price=request.price;
        if(request.price<0){result.code=ShopItemPurchaseCode::InvalidPrice;return result;}
        if(!request.item.Valid()){result.code=ShopItemPurchaseCode::InvalidItem;return result;}
        WalletSnapshot wallet;
        if(!walletQuery_.TryGetWallet(request.player,wallet)){result.code=ShopItemPurchaseCode::WalletNotFound;return result;}
        result.before=wallet.gold;result.after=wallet.gold;
        if(wallet.gold<request.price){result.code=ShopItemPurchaseCode::InsufficientFunds;return result;}
        if(inventoryQuery_.Contains(request.player,request.item).Present()){result.code=ShopItemPurchaseCode::InventoryRejected;return result;}
        const inventory::InventoryMutationResult grant=inventoryCommands_.Add(request.player,request.item);
        if(grant.code!=inventory::InventoryMutationCode::Applied){result.code=ShopItemPurchaseCode::InventoryRejected;return result;}
        if(request.price==0){result.code=ShopItemPurchaseCode::Purchased;return result;}
        const WalletMutationResult debit=walletCommands_.Debit(request.player,request.price,WalletReason::NpcBuyItem);
        result.after=debit.after;
        if(!debit.Ok())
        {
            const inventory::InventoryMutationResult rollback=inventoryCommands_.Remove(request.player,request.item);
            result.code=rollback.code==inventory::InventoryMutationCode::Applied?ShopItemPurchaseCode::WalletRejected:ShopItemPurchaseCode::RollbackFailed;
            return result;
        }
        result.code=ShopItemPurchaseCode::Purchased;
        return result;
    }

    ShopItemSaleResult SellItem(const ShopItemSaleRequest& request) override
    {
        static const CurrencyAmount kWalletMax=1000000000LL;
        ShopItemSaleResult result;
        result.player=request.player;result.item=request.item;result.amount=request.amount;
        if(request.amount<0||request.amount>kWalletMax){result.code=ShopItemSaleCode::InvalidAmount;return result;}
        if(!request.item.Valid()){result.code=ShopItemSaleCode::InvalidItem;return result;}
        WalletSnapshot wallet;
        if(!walletQuery_.TryGetWallet(request.player,wallet)){result.code=ShopItemSaleCode::WalletNotFound;return result;}
        result.before=wallet.gold;result.after=wallet.gold;
        if(wallet.gold<0||wallet.gold>kWalletMax-request.amount){result.code=ShopItemSaleCode::CapacityExceeded;return result;}
        inventory::InventoryReservationRequest reservationRequest;reservationRequest.owner=request.player;reservationRequest.items.push_back(request.item);
        const inventory::InventoryReservationResult reservation=inventoryReservations_.Reserve(reservationRequest);
        if(!reservation.Reserved()){result.code=ShopItemSaleCode::InventoryReservationRejected;return result;}
        ReservationGuard reservationGuard(inventoryReservations_,reservation.token);
        const inventory::InventoryMutationResult removed=inventoryCommands_.Remove(request.player,request.item);
        if(removed.code!=inventory::InventoryMutationCode::Applied){result.code=ShopItemSaleCode::ItemNotFound;return result;}
        if(request.amount==0){result.code=ShopItemSaleCode::Sold;return result;}
        const WalletMutationResult credit=walletCommands_.Credit(request.player,request.amount,WalletReason::NpcSellItem);
        result.after=credit.after;
        if(!credit.Ok())
        {
            const inventory::InventoryMutationResult rollback=inventoryCommands_.Add(request.player,request.item);
            result.code=rollback.Applied()?ShopItemSaleCode::WalletRejected:ShopItemSaleCode::RollbackFailed;
            return result;
        }
        result.code=ShopItemSaleCode::Sold;
        return result;
    }

    ShopSaleResult CreditSale(const ShopSaleRequest& request) override
    {
        static const CurrencyAmount kWalletMax=1000000000LL;
        ShopSaleResult result;result.player=request.player;result.amount=request.amount;
        if(request.amount<0||request.amount>kWalletMax){result.code=ShopSaleCode::InvalidAmount;return result;}
        WalletSnapshot wallet;
        if(!walletQuery_.TryGetWallet(request.player,wallet)){result.code=ShopSaleCode::WalletNotFound;return result;}
        result.before=wallet.gold;result.after=wallet.gold;
        if(request.amount==0){result.code=ShopSaleCode::Credited;return result;}
        if(wallet.gold<0||wallet.gold>kWalletMax-request.amount){result.code=ShopSaleCode::CapacityExceeded;return result;}
        const WalletMutationResult credit=walletCommands_.Credit(request.player,request.amount,WalletReason::NpcSellItem);
        result.after=credit.after;result.code=credit.Ok()?ShopSaleCode::Credited:ShopSaleCode::WalletRejected;return result;
    }

    ShopSaleResult RollbackSale(const ShopSaleRequest& request) override
    {
        ShopSaleResult result;result.player=request.player;result.amount=request.amount;
        if(request.amount<0){result.code=ShopSaleCode::InvalidAmount;return result;}
        WalletSnapshot wallet;
        if(!walletQuery_.TryGetWallet(request.player,wallet)){result.code=ShopSaleCode::WalletNotFound;return result;}
        result.before=wallet.gold;result.after=wallet.gold;
        if(request.amount==0){result.code=ShopSaleCode::RolledBack;return result;}
        const WalletMutationResult debit=walletCommands_.Debit(request.player,request.amount,WalletReason::NpcSellItem);
        result.after=debit.after;result.code=debit.Ok()?ShopSaleCode::RolledBack:ShopSaleCode::RollbackRejected;return result;
    }

private:
    class ReservationGuard
    {
    public:
        ReservationGuard(inventory::IInventoryReservationService& service, inventory::InventoryReservationToken token)
            : service_(service),token_(token){}
        ~ReservationGuard(){if(token_.Valid())(void)service_.Release(token_);}
    private:
        inventory::IInventoryReservationService& service_; inventory::InventoryReservationToken token_;
    };
    const IWalletQuery& walletQuery_; IWalletCommands& walletCommands_;
    const inventory::IInventoryQuery& inventoryQuery_; inventory::IInventoryCommands& inventoryCommands_;
    inventory::IInventoryReservationService& inventoryReservations_;
    const inventory::IPotionStackQuery& potionStackQuery_; inventory::IPotionStackCommands& potionStackCommands_;
};
}
}
