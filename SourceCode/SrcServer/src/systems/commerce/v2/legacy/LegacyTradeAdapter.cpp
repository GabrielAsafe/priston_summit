#include "systems/commerce/v2/legacy/LegacyTradeAdapter.h"

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"
#include "systems/commerce/coin_shop/CoinShop.h"
#include "systems/commerce/v2/legacy/LegacyWalletAdapter.h"
#include "systems/commerce/v2/legacy/LegacyCoinWalletAdapter.h"
#include "systems/inventory/v2/legacy/LegacyInventoryRuntime.h"
#include "systems/gameplay/inventory/v2/legacy/LegacyPotionStackAdapter.h"

namespace gameplay
{
namespace commerce
{
namespace legacy
{
namespace
{
TradeIngressResult BaseResult(const TradeIngressRequest& request)
{
    TradeIngressResult result;
    result.sender = request.sender;
    result.receiver = request.receiver;
    result.action = request.action;
    result.legacyFlag = request.legacyFlag;
    return result;
}
}

LegacyTradeAdapter::LegacyTradeAdapter(const player::legacy::LegacyOnlinePlayerRegistry& registry,
                                           const IWalletQuery& walletQuery,
                                           IWalletCommands& walletCommands,
                                           inventory::IInventoryCommands& inventoryCommands,
                                           const ICoinWalletQuery& coinQuery,
                                           ICoinWalletCommands& coinCommands,
                                           inventory::IPotionStackCommands& potionStackCommands)
    : registry_(registry), walletQuery_(walletQuery), walletCommands_(walletCommands),
      inventoryCommands_(inventoryCommands), coinQuery_(coinQuery), coinCommands_(coinCommands),
      potionStackCommands_(potionStackCommands)
{
}

TradeIngressResult LegacyTradeAdapter::EvaluateIngress(const TradeIngressRequest& request)
{
    TradeIngressResult result = BaseResult(request);
    rsPLAYINFO* sender = registry_.Resolve(request.sender);
    if(!sender)
    {
        result.code = TradeIngressCode::SenderNotFound;
        return result;
    }

    rsPLAYINFO* receiver = registry_.Resolve(request.receiver);
    if(!receiver)
    {
        result.code = TradeIngressCode::ReceiverNotFound;
        return result;
    }

    if(sender->dwFuncChkErrorFlag || receiver->dwFuncChkErrorFlag)
    {
        result.code = TradeIngressCode::FunctionCheckBlocked;
        return result;
    }

    if(sender->szServerID[0])
    {
        result.code = TradeIngressCode::ServerModeBlocked;
        return result;
    }

    if(rsServerConfig.DisableThrowItem)
    {
        result.code = TradeIngressCode::TradingDisabled;
        return result;
    }

    // Preserve the exact legacy ordering: coin-shop delivery is flushed for
    // both participants only after every trade blocker has passed and before
    // the unchanged wire packet is relayed by the network ingress handler.
    cCoinShop.RecvGetItemCoin(sender);
    cCoinShop.RecvGetItemCoin(receiver);

    result.code = TradeIngressCode::Relay;
    return result;
}

TradeCommitGuardResult LegacyTradeAdapter::CheckCommitReplay(const TradeCommitGuardRequest& request)
{
    static const std::uint32_t kLegacyDuplicateWindowMs = 8000u;

    TradeCommitGuardResult result;
    result.tradeId = BuildTradeId(request.fingerprint);

    rsPLAYINFO* sender = registry_.Resolve(request.fingerprint.sender);
    if(!sender)
    {
        result.code = TradeCommitGuardCode::SenderNotFound;
        return result;
    }

    const std::uint32_t elapsed = request.nowMs - static_cast<std::uint32_t>(sender->dwLastTradeTime);
    result.code = elapsed < kLegacyDuplicateWindowMs &&
                  static_cast<std::uint32_t>(sender->dwLastTradeChksum) == result.tradeId.value
                      ? TradeCommitGuardCode::Duplicate
                      : TradeCommitGuardCode::Fresh;
    return result;
}

void LegacyTradeAdapter::MarkCommitted(const TradeCommitGuardRequest& request)
{
    rsPLAYINFO* sender = registry_.Resolve(request.fingerprint.sender);
    if(!sender)
        return;

    sender->dwLastTradeTime = static_cast<DWORD>(request.nowMs);
    sender->dwLastTradeChksum = static_cast<DWORD>(BuildTradeId(request.fingerprint).value);
}

TradeGoldCommitResult LegacyTradeAdapter::CommitGold(const TradeGoldCommitRequest& request)
{
    static const CurrencyAmount kLegacyWalletMax = 1000000000LL;

    TradeGoldCommitResult result;
    result.sender = request.sender;
    result.receiver = request.receiver;
    result.amount = request.amount;

    if(request.amount <= 0 || request.amount > kLegacyWalletMax)
    {
        result.code = TradeGoldCommitCode::InvalidAmount;
        return result;
    }

    WalletSnapshot senderWallet;
    if(!walletQuery_.TryGetWallet(request.sender, senderWallet))
    {
        result.code = TradeGoldCommitCode::SenderWalletNotFound;
        return result;
    }
    result.senderBefore = senderWallet.gold;
    result.senderAfter = senderWallet.gold;

    WalletSnapshot receiverWallet;
    if(!walletQuery_.TryGetWallet(request.receiver, receiverWallet))
    {
        result.code = TradeGoldCommitCode::ReceiverWalletNotFound;
        return result;
    }
    result.receiverBefore = receiverWallet.gold;
    result.receiverAfter = receiverWallet.gold;

    if(senderWallet.gold < request.amount)
    {
        result.code = TradeGoldCommitCode::InsufficientFunds;
        return result;
    }
    if(receiverWallet.gold < 0 || receiverWallet.gold > kLegacyWalletMax - request.amount)
    {
        result.code = TradeGoldCommitCode::ReceiverCapacityExceeded;
        return result;
    }

    const WalletMutationResult debit =
        walletCommands_.Debit(request.sender, request.amount, WalletReason::Trade);
    result.senderAfter = debit.after;
    if(!debit.Ok())
    {
        result.code = TradeGoldCommitCode::DebitRejected;
        return result;
    }

    const WalletMutationResult credit =
        walletCommands_.Credit(request.receiver, request.amount, WalletReason::Trade);
    result.receiverAfter = credit.after;
    if(!credit.Ok())
    {
        const WalletMutationResult rollback =
            walletCommands_.Credit(request.sender, request.amount, WalletReason::Trade);
        result.senderAfter = rollback.after;
        result.code = rollback.Ok()
                          ? TradeGoldCommitCode::CreditRejected
                          : TradeGoldCommitCode::RollbackFailed;
        return result;
    }

    result.code = TradeGoldCommitCode::Committed;
    return result;
}

TradeCoinCommitResult LegacyTradeAdapter::CommitCoins(const TradeCoinCommitRequest& request)
{
    static const CoinAmount kLegacyCoinMax = 2147483647LL;

    TradeCoinCommitResult result;
    result.sender = request.sender;
    result.receiver = request.receiver;
    result.amount = request.amount;
    if(request.amount <= 0 || request.amount > kLegacyCoinMax)
    {
        result.code = TradeCoinCommitCode::InvalidAmount;
        return result;
    }

    CoinWalletSnapshot senderWallet;
    if(!coinQuery_.TryGetCoinWallet(request.sender, senderWallet))
    {
        result.code = TradeCoinCommitCode::SenderWalletNotFound;
        return result;
    }
    result.senderBefore = senderWallet.coins;
    result.senderAfter = senderWallet.coins;

    CoinWalletSnapshot receiverWallet;
    if(!coinQuery_.TryGetCoinWallet(request.receiver, receiverWallet))
    {
        result.code = TradeCoinCommitCode::ReceiverWalletNotFound;
        return result;
    }
    result.receiverBefore = receiverWallet.coins;
    result.receiverAfter = receiverWallet.coins;

    if(senderWallet.coins < request.amount)
    {
        result.code = TradeCoinCommitCode::InsufficientFunds;
        return result;
    }
    if(receiverWallet.coins < 0 || receiverWallet.coins > kLegacyCoinMax - request.amount)
    {
        result.code = TradeCoinCommitCode::ReceiverCapacityExceeded;
        return result;
    }

    const CoinMutationResult debit =
        coinCommands_.DebitCoins(request.sender, request.amount, CoinReason::Trade);
    result.senderAfter = debit.after;
    if(!debit.Ok())
    {
        result.code = TradeCoinCommitCode::DebitRejected;
        return result;
    }

    const CoinMutationResult credit =
        coinCommands_.CreditCoins(request.receiver, request.amount, CoinReason::Trade);
    result.receiverAfter = credit.after;
    if(!credit.Ok())
    {
        const CoinMutationResult rollback =
            coinCommands_.CreditCoins(request.sender, request.amount, CoinReason::Trade);
        result.senderAfter = rollback.after;
        result.code = rollback.Ok()
                          ? TradeCoinCommitCode::CreditRejected
                          : TradeCoinCommitCode::RollbackFailed;
        return result;
    }

    result.code = TradeCoinCommitCode::Committed;
    return result;
}

TradeExactCommitResult LegacyTradeAdapter::CommitExactTrade(const TradeExactCommitRequest& request)
{
    static const CurrencyAmount kLegacyGoldMax = 1000000000LL;
    static const CoinAmount kLegacyCoinMax = 2147483647LL;

    TradeExactCommitResult result;
    if(!request.sender.Valid() || !request.receiver.Valid() || request.sender == request.receiver ||
       request.gold < 0 || request.gold > kLegacyGoldMax ||
       request.coins < 0 || request.coins > kLegacyCoinMax)
    {
        result.code = TradeExactCommitCode::InvalidRequest;
        return result;
    }

    std::array<bool, kTradeOfferItemSlots> itemTransferred = {};
    std::array<bool, kTradeOfferItemSlots> potionTransferred = {};
    const auto rollbackAssets = [&]() -> bool
    {
        bool ok = true;
        for(std::size_t index = kTradeOfferItemSlots; index > 0; --index)
        {
            const std::size_t slot = index - 1u;
            if(potionTransferred[slot])
            {
                const TradePotionStackOffer& offer = request.potionStacks[slot];
                const inventory::PotionStackTransferResult rollback = potionStackCommands_.TransferStack(
                    inventory::PotionStackTransferRequest(
                        request.receiver, request.sender, offer.potion, offer.quantity));
                ok = rollback.Applied() && ok;
                potionTransferred[slot] = false;
            }
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
        const TradeGoldCommitResult gold =
            CommitGold(TradeGoldCommitRequest(request.sender, request.receiver, request.gold));
        if(!gold.Committed())
        {
            const bool assetRollbackOk = rollbackAssets();
            result.code = gold.code == TradeGoldCommitCode::RollbackFailed || !assetRollbackOk
                              ? TradeExactCommitCode::RollbackFailed
                              : TradeExactCommitCode::GoldRejected;
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
            walletCommands_.Credit(request.receiver, request.gold, WalletReason::Trade);
            return false;
        }
        goldCommitted = false;
        return true;
    };

    if(request.coins > 0)
    {
        const TradeCoinCommitResult coins =
            CommitCoins(TradeCoinCommitRequest(request.sender, request.receiver, request.coins));
        if(!coins.Committed())
        {
            const bool goldRollbackOk = rollbackGold();
            const bool assetRollbackOk = rollbackAssets();
            result.code = coins.code == TradeCoinCommitCode::RollbackFailed ||
                          !goldRollbackOk || !assetRollbackOk
                              ? TradeExactCommitCode::RollbackFailed
                              : TradeExactCommitCode::CoinRejected;
            return result;
        }
    }

    result.code = TradeExactCommitCode::Committed;
    return result;
}

inventory::InventoryTransferResult LegacyTradeAdapter::TransferItem(
    const TradeItemTransferRequest& request)
{
    return inventoryCommands_.Transfer(request.sender, request.receiver, request.item);
}

inventory::InventoryMutationResult LegacyTradeAdapter::RemoveItem(
    PlayerId player,
    inventory::ItemInstanceId item)
{
    return inventoryCommands_.Remove(player, item);
}

LegacyTradeAdapter& ServerLegacyTradeAdapter()
{
    LegacyWalletAdapter& wallet = ServerLegacyWalletAdapter();
    inventory::legacy::LegacyInventoryRuntime& inventory = inventory::legacy::ServerLegacyInventoryRuntime();
    LegacyCoinWalletAdapter& coin = ServerLegacyCoinWalletAdapter();
    inventory::legacy::LegacyPotionStackAdapter& potionStacks =
        inventory::legacy::ServerLegacyPotionStackAdapter();
    static LegacyTradeAdapter adapter(
        player::legacy::ServerLegacyOnlinePlayerRegistry(), wallet, wallet, inventory, coin, coin, potionStacks);
    return adapter;
}
}
}
}
