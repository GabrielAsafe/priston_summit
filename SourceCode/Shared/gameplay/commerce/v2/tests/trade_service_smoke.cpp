#include "gameplay/commerce/v2/TradeService.h"

#include <cassert>
#include <iostream>

namespace
{
class FakeTradeService final : public gameplay::commerce::ITradeService
{
public:
    bool marked = false;
    gameplay::commerce::TradeIngressResult EvaluateIngress(
        const gameplay::commerce::TradeIngressRequest& request) override
    {
        gameplay::commerce::TradeIngressResult result;
        result.sender = request.sender;
        result.receiver = request.receiver;
        result.action = request.action;
        result.legacyFlag = request.legacyFlag;
        result.code = request.sender.Valid() && request.receiver.Valid()
                          ? gameplay::commerce::TradeIngressCode::Relay
                          : gameplay::commerce::TradeIngressCode::SenderNotFound;
        return result;
    }

    gameplay::commerce::TradeCommitGuardResult CheckCommitReplay(
        const gameplay::commerce::TradeCommitGuardRequest& request) override
    {
        gameplay::commerce::TradeCommitGuardResult result;
        result.tradeId = gameplay::commerce::BuildTradeId(request.fingerprint);
        result.code = request.nowMs == 100u
                          ? gameplay::commerce::TradeCommitGuardCode::Duplicate
                          : gameplay::commerce::TradeCommitGuardCode::Fresh;
        return result;
    }

    void MarkCommitted(const gameplay::commerce::TradeCommitGuardRequest&) override
    {
        marked = true;
    }

    gameplay::commerce::TradeGoldCommitResult CommitGold(
        const gameplay::commerce::TradeGoldCommitRequest& request) override
    {
        gameplay::commerce::TradeGoldCommitResult result;
        result.sender = request.sender;
        result.receiver = request.receiver;
        result.amount = request.amount;
        result.senderBefore = 500;
        result.senderAfter = 500 - request.amount;
        result.receiverBefore = 100;
        result.receiverAfter = 100 + request.amount;
        result.code = request.amount > 0
                          ? gameplay::commerce::TradeGoldCommitCode::Committed
                          : gameplay::commerce::TradeGoldCommitCode::InvalidAmount;
        return result;
    }
    gameplay::commerce::TradeCoinCommitResult CommitCoins(
        const gameplay::commerce::TradeCoinCommitRequest& request) override
    {
        gameplay::commerce::TradeCoinCommitResult result;
        result.sender = request.sender;
        result.receiver = request.receiver;
        result.amount = request.amount;
        result.senderBefore = 50;
        result.senderAfter = 50 - request.amount;
        result.receiverBefore = 5;
        result.receiverAfter = 5 + request.amount;
        result.code = request.amount > 0 && request.amount <= 50
                          ? gameplay::commerce::TradeCoinCommitCode::Committed
                          : gameplay::commerce::TradeCoinCommitCode::InsufficientFunds;
        return result;
    }

    gameplay::commerce::TradeExactCommitResult CommitExactTrade(
        const gameplay::commerce::TradeExactCommitRequest& request) override
    {
        gameplay::commerce::TradeExactCommitResult result;
        result.code = request.sender.Valid() && request.receiver.Valid() && request.sender != request.receiver
                          ? gameplay::commerce::TradeExactCommitCode::Committed
                          : gameplay::commerce::TradeExactCommitCode::InvalidRequest;
        return result;
    }

    gameplay::inventory::InventoryTransferResult TransferItem(
        const gameplay::commerce::TradeItemTransferRequest& request) override
    {
        gameplay::inventory::InventoryTransferResult result;
        result.source = request.sender;
        result.destination = request.receiver;
        result.item = request.item;
        result.code = request.sender.Valid() && request.receiver.Valid() && request.item.Valid()
                          ? gameplay::inventory::InventoryTransferCode::Applied
                          : gameplay::inventory::InventoryTransferCode::InvalidItem;
        return result;
    }

    gameplay::inventory::InventoryMutationResult RemoveItem(
        gameplay::PlayerId player,
        gameplay::inventory::ItemInstanceId item) override
    {
        gameplay::inventory::InventoryMutationResult result;
        result.player = player;
        result.item = item;
        result.code = player.Valid() && item.Valid()
                          ? gameplay::inventory::InventoryMutationCode::Applied
                          : gameplay::inventory::InventoryMutationCode::InvalidItem;
        return result;
    }

};
}

int main()
{
    using namespace gameplay;
    using namespace gameplay::commerce;

    assert(TradeActionFromLegacyFlag(0) == TradeAction::Request);
    assert(TradeActionFromLegacyFlag(1) == TradeAction::Accept);
    assert(TradeActionFromLegacyFlag(2) == TradeAction::Open);
    assert(TradeActionFromLegacyFlag(3) == TradeAction::Cancel);
    assert(TradeActionFromLegacyFlag(99) == TradeAction::Unknown);

    FakeTradeService service;
    const TradeIngressRequest request(PlayerId(10u), PlayerId(20u), TradeAction::Accept, 1);
    const TradeIngressResult result = service.EvaluateIngress(request);
    assert(result.ShouldRelay());
    assert(result.sender == PlayerId(10u));
    assert(result.receiver == PlayerId(20u));
    assert(result.action == TradeAction::Accept);
    assert(result.legacyFlag == 1);

    TradeCommitGuardRequest guardRequest;
    guardRequest.fingerprint.sender = PlayerId(10u);
    guardRequest.fingerprint.receiver = PlayerId(20u);
    guardRequest.fingerprint.encodedMoney = 193;
    guardRequest.fingerprint.items[0].itemCode = 100u;
    guardRequest.fingerprint.items[0].itemKey = 200u;
    guardRequest.fingerprint.items[0].itemChecksum = 300u;
    guardRequest.nowMs = 101u;
    const TradeId expectedId(static_cast<std::uint32_t>(193u * 10u + 20u + (100u + 200u + 300u) * 2u));
    assert(BuildTradeId(guardRequest.fingerprint) == expectedId);
    const TradeCommitGuardResult fresh = service.CheckCommitReplay(guardRequest);
    assert(fresh.Fresh());
    assert(fresh.tradeId == expectedId);
    guardRequest.nowMs = 100u;
    assert(service.CheckCommitReplay(guardRequest).Duplicate());
    service.MarkCommitted(guardRequest);
    assert(service.marked);

    const TradeGoldCommitResult gold =
        service.CommitGold(TradeGoldCommitRequest(PlayerId(10u), PlayerId(20u), 125));
    assert(gold.Committed());
    assert(gold.senderBefore == 500);
    assert(gold.senderAfter == 375);
    assert(gold.receiverBefore == 100);
    assert(gold.receiverAfter == 225);

    const TradeCoinCommitResult coins =
        service.CommitCoins(TradeCoinCommitRequest(PlayerId(10u), PlayerId(20u), 20));
    assert(coins.Committed());
    assert(coins.senderAfter == 30);
    assert(coins.receiverAfter == 25);

    TradeExactCommitRequest exact;
    exact.sender = PlayerId(10u);
    exact.receiver = PlayerId(20u);
    exact.gold = 10;
    exact.coins = 2;
    exact.items[0] = inventory::ItemInstanceId(ItemId(0x2222u), 1u, 2u);
    exact.potionStacks[1] = TradePotionStackOffer(ItemId(0x3333u), 5);
    assert(service.CommitExactTrade(exact).Committed());

    const inventory::ItemInstanceId item(ItemId(0x1111u), 22u, 33u);
    assert(service.TransferItem(TradeItemTransferRequest(PlayerId(10u), PlayerId(20u), item)).Applied());
    assert(service.RemoveItem(PlayerId(10u), item).Applied());

    std::cout << "trade service smoke: PASS\n";
    return 0;
}
