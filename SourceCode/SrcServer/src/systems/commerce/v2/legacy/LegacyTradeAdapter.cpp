#include "systems/commerce/v2/legacy/LegacyTradeAdapter.h"

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"
#include "systems/commerce/coin_shop/CoinShop.h"
#include "systems/commerce/v2/legacy/LegacyWalletAdapter.h"
#include "systems/commerce/v2/legacy/LegacyCoinWalletAdapter.h"
#include "systems/inventory/v2/legacy/LegacyInventoryRuntime.h"
#include "systems/inventory/v2/legacy/LegacyInventoryReservationRuntime.h"
#include "systems/gameplay/inventory/v2/legacy/LegacyPotionStackAdapter.h"
#include "gameplay/commerce/v2/TradeSessionService.h"
#include "gameplay/commerce/v2/TradeTransactionService.h"

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
                                           inventory::IInventoryReservationService& inventoryReservations,
                                           const ICoinWalletQuery& coinQuery,
                                           ICoinWalletCommands& coinCommands,
                                           inventory::IPotionStackCommands& potionStackCommands,
                                           ITradeReplayGuard& replayGuard,
                                           ITradeTransactionService& transactionService)
    : registry_(registry), walletQuery_(walletQuery), walletCommands_(walletCommands),
      inventoryCommands_(inventoryCommands), inventoryReservations_(inventoryReservations),
      coinQuery_(coinQuery), coinCommands_(coinCommands),
      potionStackCommands_(potionStackCommands), replayGuard_(replayGuard),
      transactionService_(transactionService)
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
    rsPLAYINFO* sender = registry_.Resolve(request.fingerprint.sender);
    if(!sender)
    {
        TradeCommitGuardResult result;
        result.tradeId = BuildTradeId(request.fingerprint);
        result.code = TradeCommitGuardCode::SenderNotFound;
        return result;
    }

    return replayGuard_.Check(
        registry_.HandleOf(sender), request.fingerprint, request.nowMs);
}

void LegacyTradeAdapter::MarkCommitted(const TradeCommitGuardRequest& request)
{
    rsPLAYINFO* sender = registry_.Resolve(request.fingerprint.sender);
    if(!sender)
        return;

    replayGuard_.MarkCommitted(
        registry_.HandleOf(sender), request.fingerprint, request.nowMs);
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
    return transactionService_.CommitExactTrade(request);
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
    static TradeReplayGuardService replayGuard(8000u);
    static TradeTransactionService transactionService(
        wallet, wallet, inventory, inventory::legacy::ServerInventoryReservationService(),
        coin, coin, potionStacks);
    static LegacyTradeAdapter adapter(
        player::legacy::ServerLegacyOnlinePlayerRegistry(), wallet, wallet, inventory,
        inventory::legacy::ServerInventoryReservationService(),
        coin, coin, potionStacks, replayGuard, transactionService);
    return adapter;
}
}
}
}
