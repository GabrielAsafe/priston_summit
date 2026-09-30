#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "gameplay/core/v2/GameplayIds.h"
#include "gameplay/commerce/v2/Wallet.h"
#include "gameplay/commerce/v2/CoinWallet.h"
#include "gameplay/inventory/v2/Inventory.h"
#include "gameplay/inventory/v2/PotionStack.h"

namespace gameplay
{
namespace commerce
{
struct TradeId
{
    std::uint32_t value;
    explicit constexpr TradeId(std::uint32_t valueValue = 0u) : value(valueValue) {}
    constexpr bool Valid() const { return value != 0u; }
};

inline constexpr bool operator==(TradeId lhs, TradeId rhs) { return lhs.value == rhs.value; }
inline constexpr bool operator!=(TradeId lhs, TradeId rhs) { return !(lhs == rhs); }

constexpr std::size_t kTradeOfferItemSlots = 20u;

struct TradeItemFingerprint
{
    std::uint32_t itemCode;
    std::uint32_t itemKey;
    std::uint32_t itemChecksum;

    TradeItemFingerprint() : itemCode(0u), itemKey(0u), itemChecksum(0u) {}
};

struct TradeCommitFingerprint
{
    PlayerId sender;
    PlayerId receiver;
    std::int32_t encodedMoney;
    std::array<TradeItemFingerprint, kTradeOfferItemSlots> items;

    TradeCommitFingerprint() : sender(), receiver(), encodedMoney(0), items() {}
};

inline TradeId BuildTradeId(const TradeCommitFingerprint& fingerprint)
{
    // Preserve the legacy DWORD checksum arithmetic, including wraparound.
    std::uint32_t value =
        static_cast<std::uint32_t>(fingerprint.encodedMoney) * fingerprint.sender.value +
        fingerprint.receiver.value;
    for(std::size_t index = 0; index < fingerprint.items.size(); ++index)
    {
        const TradeItemFingerprint& item = fingerprint.items[index];
        if(item.itemCode == 0u)
            continue;
        value += (item.itemCode + item.itemKey + item.itemChecksum) *
                 static_cast<std::uint32_t>(index + 2u);
    }
    return TradeId(value);
}

enum class TradeCommitGuardCode
{
    Fresh,
    Duplicate,
    SenderNotFound
};

struct TradeCommitGuardRequest
{
    TradeCommitFingerprint fingerprint;
    std::uint32_t nowMs;

    TradeCommitGuardRequest() : fingerprint(), nowMs(0u) {}
};

struct TradeCommitGuardResult
{
    TradeCommitGuardCode code;
    TradeId tradeId;

    TradeCommitGuardResult() : code(TradeCommitGuardCode::SenderNotFound), tradeId() {}
    bool Fresh() const { return code == TradeCommitGuardCode::Fresh; }
    bool Duplicate() const { return code == TradeCommitGuardCode::Duplicate; }
};


struct TradeItemTransferRequest
{
    PlayerId sender;
    PlayerId receiver;
    inventory::ItemInstanceId item;

    TradeItemTransferRequest(PlayerId senderValue = PlayerId(),
                             PlayerId receiverValue = PlayerId(),
                             inventory::ItemInstanceId itemValue = inventory::ItemInstanceId())
        : sender(senderValue), receiver(receiverValue), item(itemValue) {}
};


enum class TradeExactCommitCode
{
    Committed,
    InvalidRequest,
    ItemTransferRejected,
    PotionTransferRejected,
    GoldRejected,
    CoinRejected,
    RollbackFailed
};


struct TradePotionStackOffer
{
    ItemId potion;
    inventory::PotionQuantity quantity;

    TradePotionStackOffer(ItemId potionValue = ItemId(), inventory::PotionQuantity quantityValue = 0)
        : potion(potionValue), quantity(quantityValue) {}
};

struct TradeExactCommitRequest
{
    PlayerId sender;
    PlayerId receiver;
    CurrencyAmount gold;
    CoinAmount coins;
    std::array<inventory::ItemInstanceId, kTradeOfferItemSlots> items;
    std::array<TradePotionStackOffer, kTradeOfferItemSlots> potionStacks;

    TradeExactCommitRequest()
        : sender(), receiver(), gold(0), coins(0), items(), potionStacks() {}
};

struct TradeExactCommitResult
{
    TradeExactCommitCode code;
    std::size_t failingItemIndex;

    TradeExactCommitResult()
        : code(TradeExactCommitCode::InvalidRequest), failingItemIndex(kTradeOfferItemSlots) {}

    bool Committed() const { return code == TradeExactCommitCode::Committed; }
};

enum class TradeAction
{
    Request = 0,
    Accept = 1,
    Open = 2,
    Cancel = 3,
    Unknown = 255
};

inline TradeAction TradeActionFromLegacyFlag(int flag)
{
    switch(flag)
    {
        case 0: return TradeAction::Request;
        case 1: return TradeAction::Accept;
        case 2: return TradeAction::Open;
        case 3: return TradeAction::Cancel;
        default: return TradeAction::Unknown;
    }
}

enum class TradeIngressCode
{
    Relay,
    SenderNotFound,
    ReceiverNotFound,
    FunctionCheckBlocked,
    ServerModeBlocked,
    TradingDisabled
};

struct TradeIngressRequest
{
    PlayerId sender;
    PlayerId receiver;
    TradeAction action;
    int legacyFlag;

    TradeIngressRequest(PlayerId senderValue = PlayerId(),
                        PlayerId receiverValue = PlayerId(),
                        TradeAction actionValue = TradeAction::Unknown,
                        int legacyFlagValue = 0)
        : sender(senderValue), receiver(receiverValue), action(actionValue), legacyFlag(legacyFlagValue)
    {
    }
};


enum class TradeCoinCommitCode
{
    Committed,
    InvalidAmount,
    SenderWalletNotFound,
    ReceiverWalletNotFound,
    InsufficientFunds,
    ReceiverCapacityExceeded,
    DebitRejected,
    CreditRejected,
    RollbackFailed
};

struct TradeCoinCommitRequest
{
    PlayerId sender;
    PlayerId receiver;
    CoinAmount amount;

    TradeCoinCommitRequest(PlayerId senderValue = PlayerId(),
                           PlayerId receiverValue = PlayerId(),
                           CoinAmount amountValue = 0)
        : sender(senderValue), receiver(receiverValue), amount(amountValue) {}
};

struct TradeCoinCommitResult
{
    TradeCoinCommitCode code;
    PlayerId sender;
    PlayerId receiver;
    CoinAmount amount;
    CoinAmount senderBefore;
    CoinAmount senderAfter;
    CoinAmount receiverBefore;
    CoinAmount receiverAfter;

    TradeCoinCommitResult()
        : code(TradeCoinCommitCode::InvalidAmount), amount(0), senderBefore(0), senderAfter(0),
          receiverBefore(0), receiverAfter(0) {}

    bool Committed() const { return code == TradeCoinCommitCode::Committed; }
};

enum class TradeGoldCommitCode
{
    Committed,
    InvalidAmount,
    SenderWalletNotFound,
    ReceiverWalletNotFound,
    InsufficientFunds,
    ReceiverCapacityExceeded,
    DebitRejected,
    CreditRejected,
    RollbackFailed
};

struct TradeGoldCommitRequest
{
    PlayerId sender;
    PlayerId receiver;
    CurrencyAmount amount;

    TradeGoldCommitRequest(PlayerId senderValue = PlayerId(),
                           PlayerId receiverValue = PlayerId(),
                           CurrencyAmount amountValue = 0)
        : sender(senderValue), receiver(receiverValue), amount(amountValue)
    {
    }
};

struct TradeGoldCommitResult
{
    TradeGoldCommitCode code;
    PlayerId sender;
    PlayerId receiver;
    CurrencyAmount amount;
    CurrencyAmount senderBefore;
    CurrencyAmount senderAfter;
    CurrencyAmount receiverBefore;
    CurrencyAmount receiverAfter;

    TradeGoldCommitResult()
        : code(TradeGoldCommitCode::InvalidAmount), amount(0), senderBefore(0), senderAfter(0),
          receiverBefore(0), receiverAfter(0)
    {
    }

    bool Committed() const { return code == TradeGoldCommitCode::Committed; }
};

struct TradeIngressResult
{
    TradeIngressCode code;
    PlayerId sender;
    PlayerId receiver;
    TradeAction action;
    int legacyFlag;

    TradeIngressResult()
        : code(TradeIngressCode::SenderNotFound), action(TradeAction::Unknown), legacyFlag(0)
    {
    }

    bool ShouldRelay() const { return code == TradeIngressCode::Relay; }
};

class ITradeService
{
public:
    virtual ~ITradeService() {}
    virtual TradeIngressResult EvaluateIngress(const TradeIngressRequest& request) = 0;
    virtual TradeCommitGuardResult CheckCommitReplay(const TradeCommitGuardRequest& request) = 0;
    virtual void MarkCommitted(const TradeCommitGuardRequest& request) = 0;
    virtual TradeGoldCommitResult CommitGold(const TradeGoldCommitRequest& request) = 0;
    virtual TradeCoinCommitResult CommitCoins(const TradeCoinCommitRequest& request) = 0;
    virtual TradeExactCommitResult CommitExactTrade(const TradeExactCommitRequest& request) = 0;
    virtual inventory::InventoryTransferResult TransferItem(const TradeItemTransferRequest& request) = 0;
    virtual inventory::InventoryMutationResult RemoveItem(PlayerId player, inventory::ItemInstanceId item) = 0;
};
}
}
