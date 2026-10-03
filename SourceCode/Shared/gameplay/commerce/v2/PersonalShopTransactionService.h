#pragma once

#include "gameplay/commerce/v2/PersonalShopTransaction.h"
#include "gameplay/commerce/v2/Wallet.h"
#include "gameplay/commerce/v2/CoinWallet.h"
#include "gameplay/inventory/v2/Inventory.h"
#include "gameplay/inventory/v2/InventoryReservation.h"
#include "gameplay/inventory/v2/PotionStack.h"

namespace gameplay
{
namespace commerce
{
class PersonalShopTransactionService final : public IPersonalShopTransactionService
{
public:
    PersonalShopTransactionService(const IWalletQuery& walletQuery,
                                   IWalletCommands& walletCommands,
                                   const inventory::IInventoryQuery& inventoryQuery,
                                   inventory::IInventoryCommands& inventoryCommands,
                                   inventory::IInventoryReservationService& inventoryReservations,
                                   const ICoinWalletQuery& coinQuery,
                                   ICoinWalletCommands& coinCommands,
                                   inventory::IPotionStackCommands& potionStackCommands)
        : walletQuery_(walletQuery), walletCommands_(walletCommands), inventoryQuery_(inventoryQuery),
          inventoryCommands_(inventoryCommands), inventoryReservations_(inventoryReservations),
          coinQuery_(coinQuery), coinCommands_(coinCommands), potionStackCommands_(potionStackCommands)
    {
    }

    PersonalShopTransactionResult Commit(const PersonalShopTransactionRequest& transaction) override
    {
        static const CurrencyAmount kWalletMax = 1000000000LL;
        static const CoinAmount kCoinMax = 0x7fffffffLL;

        PersonalShopTransactionResult output;
        PersonalShopTradeResult& result = output.trade;
        const PersonalShopTradeRequest& request = transaction.trade;
        result.request = request;

        if(!request.seller.Valid() || !request.buyer.Valid() || request.seller == request.buyer ||
           request.itemCode == 0u || request.goldPrice < 0 || request.goldPrice > kWalletMax ||
           request.coinPrice < 0 || request.coinPrice > kCoinMax ||
           (request.goldPrice == 0 && request.coinPrice == 0))
        {
            result.code = PersonalShopTradeCode::InvalidRequest;
            return output;
        }

        WalletSnapshot buyerWallet;
        WalletSnapshot sellerWallet;
        if(!walletQuery_.TryGetWallet(request.buyer, buyerWallet) || buyerWallet.gold < request.goldPrice)
        {
            result.code = PersonalShopTradeCode::InsufficientGold;
            return output;
        }
        if(!walletQuery_.TryGetWallet(request.seller, sellerWallet) || sellerWallet.gold < 0 ||
           sellerWallet.gold > kWalletMax - request.goldPrice)
        {
            result.code = PersonalShopTradeCode::SellerWalletCapacityExceeded;
            return output;
        }

        CoinWalletSnapshot buyerCoinWallet;
        CoinWalletSnapshot sellerCoinWallet;
        if(!coinQuery_.TryGetCoinWallet(request.buyer, buyerCoinWallet) ||
           buyerCoinWallet.coins < request.coinPrice)
        {
            result.code = PersonalShopTradeCode::InsufficientCoin;
            return output;
        }
        if(!coinQuery_.TryGetCoinWallet(request.seller, sellerCoinWallet) ||
           sellerCoinWallet.coins < 0 || sellerCoinWallet.coins > kCoinMax - request.coinPrice)
        {
            result.code = PersonalShopTradeCode::SellerCoinCapacityExceeded;
            return output;
        }

        const inventory::ItemInstanceId item(ItemId(request.itemCode), request.head, request.checksum);
        if(!inventoryQuery_.Contains(request.seller, item).Present())
        {
            result.code = PersonalShopTradeCode::ListingNotFound;
            return output;
        }

        inventory::InventoryReservationRequest reservationRequest;
        reservationRequest.owner = request.seller;
        reservationRequest.items.push_back(item);
        const inventory::InventoryReservationResult reservation =
            inventoryReservations_.Reserve(reservationRequest);
        if(!reservation.Reserved())
        {
            result.code = PersonalShopTradeCode::InventoryReservationRejected;
            return output;
        }
        ReservationGuard reservationGuard(inventoryReservations_, reservation.token);

        bool buyerItemAdded = false;
        bool sellerItemRemoved = false;
        bool potionMoved = false;
        bool buyerGoldDebited = false;
        bool sellerGoldCredited = false;
        bool buyerCoinDebited = false;
        bool sellerCoinCredited = false;

        const auto rollbackInventory = [&]() -> bool
        {
            bool ok = true;
            if(sellerItemRemoved)
            {
                const inventory::InventoryMutationResult restored = inventoryCommands_.Add(request.seller, item);
                ok = restored.Applied() && ok;
                sellerItemRemoved = false;
            }
            if(buyerItemAdded)
            {
                const inventory::InventoryMutationResult removed = inventoryCommands_.Remove(request.buyer, item);
                ok = removed.code == inventory::InventoryMutationCode::Applied && ok;
                buyerItemAdded = false;
            }
            if(potionMoved)
            {
                const inventory::PotionStackTransferResult potionRollback = potionStackCommands_.TransferStack(
                    inventory::PotionStackTransferRequest(
                        request.buyer, request.seller, ItemId(request.itemCode),
                        static_cast<inventory::PotionQuantity>(request.quantity)));
                ok = potionRollback.Applied() && ok;
                potionMoved = false;
            }
            return ok;
        };

        const auto rollbackGold = [&]() -> bool
        {
            bool ok = true;
            if(sellerGoldCredited)
            {
                const WalletMutationResult undoSeller =
                    walletCommands_.Debit(request.seller, request.goldPrice, WalletReason::PersonalShop);
                ok = undoSeller.Ok() && ok;
                sellerGoldCredited = false;
            }
            if(buyerGoldDebited)
            {
                const WalletMutationResult undoBuyer =
                    walletCommands_.Credit(request.buyer, request.goldPrice, WalletReason::PersonalShop);
                ok = undoBuyer.Ok() && ok;
                buyerGoldDebited = false;
            }
            return ok;
        };

        const auto rollbackCoin = [&]() -> bool
        {
            bool ok = true;
            if(sellerCoinCredited)
            {
                const CoinMutationResult undoSeller =
                    coinCommands_.DebitCoins(request.seller, request.coinPrice, CoinReason::PersonalShop);
                ok = undoSeller.Ok() && ok;
                sellerCoinCredited = false;
            }
            if(buyerCoinDebited)
            {
                const CoinMutationResult undoBuyer =
                    coinCommands_.CreditCoins(request.buyer, request.coinPrice, CoinReason::PersonalShop);
                ok = undoBuyer.Ok() && ok;
                buyerCoinDebited = false;
            }
            return ok;
        };

        if(!transaction.potion)
        {
            const inventory::InventoryMutationResult buyerAdd = inventoryCommands_.Add(request.buyer, item);
            if(buyerAdd.code != inventory::InventoryMutationCode::Applied)
            {
                result.code = PersonalShopTradeCode::BuyerInventoryRejected;
                return output;
            }
            buyerItemAdded = true;
        }
        else
        {
            const inventory::PotionStackTransferResult potionTransfer = potionStackCommands_.TransferStack(
                inventory::PotionStackTransferRequest(
                    request.seller, request.buyer, ItemId(request.itemCode),
                    static_cast<inventory::PotionQuantity>(request.quantity)));
            if(!potionTransfer.Applied())
            {
                output.potionTransferFailed = true;
                result.code = potionTransfer.code == inventory::PotionStackTransferCode::RollbackFailed
                                  ? PersonalShopTradeCode::RollbackFailed
                                  : PersonalShopTradeCode::PotionTransferRejected;
                return output;
            }
            potionMoved = true;
        }

        const inventory::InventoryMutationResult sellerRemove = inventoryCommands_.Remove(request.seller, item);
        if(sellerRemove.code != inventory::InventoryMutationCode::Applied)
        {
            result.code = rollbackInventory()
                              ? PersonalShopTradeCode::ItemDeleteRejected
                              : PersonalShopTradeCode::RollbackFailed;
            return output;
        }
        sellerItemRemoved = true;

        if(request.goldPrice > 0)
        {
            const WalletMutationResult debit =
                walletCommands_.Debit(request.buyer, request.goldPrice, WalletReason::PersonalShop);
            if(!debit.Ok())
            {
                result.code = rollbackInventory()
                                  ? PersonalShopTradeCode::WalletRejected
                                  : PersonalShopTradeCode::RollbackFailed;
                return output;
            }
            buyerGoldDebited = true;

            const WalletMutationResult credit =
                walletCommands_.Credit(request.seller, request.goldPrice, WalletReason::PersonalShop);
            if(!credit.Ok())
            {
                const bool rollbackOk = rollbackGold() && rollbackInventory();
                result.code = rollbackOk ? PersonalShopTradeCode::WalletRejected
                                         : PersonalShopTradeCode::RollbackFailed;
                return output;
            }
            sellerGoldCredited = true;
        }

        if(request.coinPrice > 0)
        {
            const CoinMutationResult debit =
                coinCommands_.DebitCoins(request.buyer, request.coinPrice, CoinReason::PersonalShop);
            if(!debit.Ok())
            {
                const bool rollbackOk = rollbackGold() && rollbackInventory();
                result.code = rollbackOk ? PersonalShopTradeCode::CoinWalletRejected
                                         : PersonalShopTradeCode::RollbackFailed;
                return output;
            }
            buyerCoinDebited = true;

            const CoinMutationResult credit =
                coinCommands_.CreditCoins(request.seller, request.coinPrice, CoinReason::PersonalShop);
            if(!credit.Ok())
            {
                const bool rollbackOk = rollbackCoin() && rollbackGold() && rollbackInventory();
                result.code = rollbackOk ? PersonalShopTradeCode::CoinWalletRejected
                                         : PersonalShopTradeCode::RollbackFailed;
                return output;
            }
            sellerCoinCredited = true;
        }

        if(transaction.potion)
        {
            // Preserve the legacy exact-item metadata bookkeeping for potion stacks.
            const inventory::InventoryMutationResult buyerAdd = inventoryCommands_.Add(request.buyer, item);
            if(!buyerAdd.Applied())
            {
                const bool rollbackOk = rollbackCoin() && rollbackGold() && rollbackInventory();
                result.code = rollbackOk ? PersonalShopTradeCode::BuyerInventoryRejected
                                         : PersonalShopTradeCode::RollbackFailed;
                return output;
            }
        }

        result.code = PersonalShopTradeCode::Committed;
        return output;
    }

private:
    class ReservationGuard
    {
    public:
        ReservationGuard(inventory::IInventoryReservationService& service,
                         inventory::InventoryReservationToken token)
            : service_(service), token_(token) {}
        ~ReservationGuard()
        {
            if(token_.Valid())
                (void)service_.Release(token_);
        }
    private:
        inventory::IInventoryReservationService& service_;
        inventory::InventoryReservationToken token_;
    };

    const IWalletQuery& walletQuery_;
    IWalletCommands& walletCommands_;
    const inventory::IInventoryQuery& inventoryQuery_;
    inventory::IInventoryCommands& inventoryCommands_;
    inventory::IInventoryReservationService& inventoryReservations_;
    const ICoinWalletQuery& coinQuery_;
    ICoinWalletCommands& coinCommands_;
    inventory::IPotionStackCommands& potionStackCommands_;
};
}
}
