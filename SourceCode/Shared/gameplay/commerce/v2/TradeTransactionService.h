#pragma once

#include <array>
#include <cstddef>

#include "gameplay/commerce/v2/TradeTransaction.h"
#include "gameplay/inventory/v2/InventoryReservation.h"

namespace gameplay
{
namespace commerce
{
class TradeTransactionService final : public ITradeTransactionService
{
public:
    TradeTransactionService(const IWalletQuery& walletQuery,
                            IWalletCommands& walletCommands,
                            inventory::IInventoryCommands& inventoryCommands,
                            inventory::IInventoryReservationService& inventoryReservations,
                            const ICoinWalletQuery& coinQuery,
                            ICoinWalletCommands& coinCommands,
                            inventory::IPotionStackCommands& potionStackCommands)
        : walletQuery_(walletQuery), walletCommands_(walletCommands),
          inventoryCommands_(inventoryCommands), inventoryReservations_(inventoryReservations),
          coinQuery_(coinQuery), coinCommands_(coinCommands), potionStackCommands_(potionStackCommands)
    {
    }

    TradeExactCommitResult CommitExactTrade(const TradeExactCommitRequest& request) override
    {
        static const CurrencyAmount kGoldMax = 1000000000LL;
        static const CoinAmount kCoinMax = 2147483647LL;

        TradeExactCommitResult result;
        if(!request.sender.Valid() || !request.receiver.Valid() || request.sender == request.receiver ||
           request.gold < 0 || request.gold > kGoldMax ||
           request.coins < 0 || request.coins > kCoinMax)
        {
            result.code = TradeExactCommitCode::InvalidRequest;
            return result;
        }

        inventory::InventoryReservationToken reservationToken;
        inventory::InventoryReservationRequest reservationRequest;
        reservationRequest.owner = request.sender;
        for(std::size_t index = 0; index < request.items.size(); ++index)
            if(request.items[index].Valid())
                reservationRequest.items.push_back(request.items[index]);

        if(!reservationRequest.items.empty())
        {
            const inventory::InventoryReservationResult reservation =
                inventoryReservations_.Reserve(reservationRequest);
            if(!reservation.Reserved())
            {
                result.code = TradeExactCommitCode::InventoryReservationRejected;
                return result;
            }
            reservationToken = reservation.token;
        }

        ReservationGuard reservationGuard(inventoryReservations_, reservationToken);
        std::array<bool, kTradeOfferItemSlots> itemTransferred = {};
        std::array<bool, kTradeOfferItemSlots> potionTransferred = {};

        const auto rollbackAssets = [&]() -> bool
        {
            bool ok = true;
            for(std::size_t index = kTradeOfferItemSlots; index > 0; --index)
            {
                const std::size_t slot = index - 1u;
                if(!potionTransferred[slot])
                    continue;
                const TradePotionStackOffer& offer = request.potionStacks[slot];
                const inventory::PotionStackTransferResult rollback = potionStackCommands_.TransferStack(
                    inventory::PotionStackTransferRequest(
                        request.receiver, request.sender, offer.potion, offer.quantity));
                ok = rollback.Applied() && ok;
                potionTransferred[slot] = false;
            }
            for(std::size_t index = kTradeOfferItemSlots; index > 0; --index)
            {
                const std::size_t slot = index - 1u;
                if(!itemTransferred[slot])
                    continue;
                const inventory::InventoryTransferResult rollback =
                    inventoryCommands_.Transfer(request.receiver, request.sender, request.items[slot]);
                ok = rollback.Applied() && ok;
                itemTransferred[slot] = false;
            }
            return ok;
        };

        for(std::size_t index = 0; index < request.items.size(); ++index)
        {
            if(!request.items[index].Valid())
                continue;
            const inventory::InventoryTransferResult transfer =
                inventoryCommands_.Transfer(request.sender, request.receiver, request.items[index]);
            if(!transfer.Applied())
            {
                result.failingItemIndex = index;
                result.code = rollbackAssets()
                                  ? TradeExactCommitCode::ItemTransferRejected
                                  : TradeExactCommitCode::RollbackFailed;
                return result;
            }
            itemTransferred[index] = true;
        }

        for(std::size_t index = 0; index < request.potionStacks.size(); ++index)
        {
            const TradePotionStackOffer& offer = request.potionStacks[index];
            if(!offer.potion.Valid())
                continue;
            if(offer.quantity <= 0)
            {
                result.failingItemIndex = index;
                result.code = rollbackAssets()
                                  ? TradeExactCommitCode::InvalidRequest
                                  : TradeExactCommitCode::RollbackFailed;
                return result;
            }
            const inventory::PotionStackTransferResult transfer = potionStackCommands_.TransferStack(
                inventory::PotionStackTransferRequest(
                    request.sender, request.receiver, offer.potion, offer.quantity));
            if(!transfer.Applied())
            {
                result.failingItemIndex = index;
                result.code = rollbackAssets()
                                  ? TradeExactCommitCode::PotionTransferRejected
                                  : TradeExactCommitCode::RollbackFailed;
                return result;
            }
            potionTransferred[index] = true;
        }

        bool goldCommitted = false;
        if(request.gold > 0)
        {
            if(!TransferGold(request.sender, request.receiver, request.gold))
            {
                result.code = rollbackAssets()
                                  ? TradeExactCommitCode::GoldRejected
                                  : TradeExactCommitCode::RollbackFailed;
                return result;
            }
            goldCommitted = true;
        }

        const auto rollbackGold = [&]() -> bool
        {
            if(!goldCommitted)
                return true;
            const WalletMutationResult undoReceiver =
                walletCommands_.Debit(request.receiver, request.gold, WalletReason::Trade);
            if(!undoReceiver.Ok())
                return false;
            const WalletMutationResult undoSender =
                walletCommands_.Credit(request.sender, request.gold, WalletReason::Trade);
            if(!undoSender.Ok())
            {
                (void)walletCommands_.Credit(request.receiver, request.gold, WalletReason::Trade);
                return false;
            }
            goldCommitted = false;
            return true;
        };

        if(request.coins > 0 && !TransferCoins(request.sender, request.receiver, request.coins))
        {
            const bool goldRollbackOk = rollbackGold();
            const bool assetRollbackOk = rollbackAssets();
            result.code = goldRollbackOk && assetRollbackOk
                              ? TradeExactCommitCode::CoinRejected
                              : TradeExactCommitCode::RollbackFailed;
            return result;
        }

        result.code = TradeExactCommitCode::Committed;
        return result;
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

    bool TransferGold(PlayerId sender, PlayerId receiver, CurrencyAmount amount)
    {
        static const CurrencyAmount kGoldMax = 1000000000LL;
        WalletSnapshot senderWallet;
        WalletSnapshot receiverWallet;
        if(!walletQuery_.TryGetWallet(sender, senderWallet) ||
           !walletQuery_.TryGetWallet(receiver, receiverWallet) ||
           senderWallet.gold < amount || receiverWallet.gold < 0 ||
           receiverWallet.gold > kGoldMax - amount)
            return false;

        const WalletMutationResult debit = walletCommands_.Debit(sender, amount, WalletReason::Trade);
        if(!debit.Ok())
            return false;
        const WalletMutationResult credit = walletCommands_.Credit(receiver, amount, WalletReason::Trade);
        if(credit.Ok())
            return true;

        (void)walletCommands_.Credit(sender, amount, WalletReason::Trade);
        return false;
    }

    bool TransferCoins(PlayerId sender, PlayerId receiver, CoinAmount amount)
    {
        static const CoinAmount kCoinMax = 2147483647LL;
        CoinWalletSnapshot senderWallet;
        CoinWalletSnapshot receiverWallet;
        if(!coinQuery_.TryGetCoinWallet(sender, senderWallet) ||
           !coinQuery_.TryGetCoinWallet(receiver, receiverWallet) ||
           senderWallet.coins < amount || receiverWallet.coins < 0 ||
           receiverWallet.coins > kCoinMax - amount)
            return false;

        const CoinMutationResult debit = coinCommands_.DebitCoins(sender, amount, CoinReason::Trade);
        if(!debit.Ok())
            return false;
        const CoinMutationResult credit = coinCommands_.CreditCoins(receiver, amount, CoinReason::Trade);
        if(credit.Ok())
            return true;

        (void)coinCommands_.CreditCoins(sender, amount, CoinReason::Trade);
        return false;
    }

    const IWalletQuery& walletQuery_;
    IWalletCommands& walletCommands_;
    inventory::IInventoryCommands& inventoryCommands_;
    inventory::IInventoryReservationService& inventoryReservations_;
    const ICoinWalletQuery& coinQuery_;
    ICoinWalletCommands& coinCommands_;
    inventory::IPotionStackCommands& potionStackCommands_;
};
}
}
