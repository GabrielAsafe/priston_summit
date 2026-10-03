#pragma once

#include <cstdint>
#include <string>

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace commerce
{
enum class PersonalShopIngressAction
{
    OpenListing,
    CallListing,
    RelayItem,
    TradeRequest
};

enum class PersonalShopIngressCode
{
    Allowed,
    SenderNotFound,
    ReceiverNotFound,
    ServerModeBlocked,
    TradingDisabled
};

struct PersonalShopIngressRequest
{
    PlayerId sender;
    PlayerId receiver;
    PersonalShopIngressAction action;
    std::string listingMessage;
    std::uint32_t listingCode;

    PersonalShopIngressRequest(PlayerId senderValue = PlayerId(),
                               PlayerId receiverValue = PlayerId(),
                               PersonalShopIngressAction actionValue = PersonalShopIngressAction::OpenListing)
        : sender(senderValue), receiver(receiverValue), action(actionValue), listingCode(0u)
    {
    }
};

struct PersonalShopIngressResult
{
    PersonalShopIngressCode code;
    PlayerId sender;
    PlayerHandle receiver;
    PersonalShopIngressAction action;

    PersonalShopIngressResult()
        : code(PersonalShopIngressCode::SenderNotFound), action(PersonalShopIngressAction::OpenListing)
    {
    }

    bool Allowed() const { return code == PersonalShopIngressCode::Allowed; }
};




enum class PersonalShopPrepareCode
{
    Prepared,
    InvalidRequest,
    SellerNotFound,
    BuyerNotFound,
    CooldownActive,
    InsufficientCurrency,
    ItemUnavailable
};

struct PersonalShopPrepareRequest
{
    PlayerId buyer;
    PlayerId seller;
    std::uint32_t itemCode;
    std::uint32_t head;
    std::uint32_t checksum;
    std::uint32_t quantity;
    std::int64_t goldPrice;
    std::int64_t coinPrice;

    PersonalShopPrepareRequest()
        : buyer(), seller(), itemCode(0u), head(0u), checksum(0u), quantity(0u), goldPrice(0), coinPrice(0) {}
};

struct PersonalShopPrepareResult
{
    PersonalShopPrepareCode code;
    PersonalShopPrepareRequest request;
    bool sendUnavailableReply;

    PersonalShopPrepareResult()
        : code(PersonalShopPrepareCode::InvalidRequest), request(), sendUnavailableReply(false) {}
    bool Prepared() const { return code == PersonalShopPrepareCode::Prepared; }
};

enum class PersonalShopMessageCode
{
    Available,
    ViewerNotFound,
    TargetNotFound,
    ServerModeBlocked,
    TradingDisabled,
    ListingCodeMismatch
};

struct PersonalShopMessageRequest
{
    PlayerId viewer;
    PlayerId target;
    std::uint32_t listingCode;

    PersonalShopMessageRequest(PlayerId viewerValue = PlayerId(),
                               PlayerId targetValue = PlayerId(),
                               std::uint32_t listingCodeValue = 0u)
        : viewer(viewerValue), target(targetValue), listingCode(listingCodeValue) {}
};

struct PersonalShopMessageResult
{
    PersonalShopMessageCode code;
    PlayerId target;
    std::string message;

    PersonalShopMessageResult() : code(PersonalShopMessageCode::ViewerNotFound), target(), message() {}
    bool Available() const { return code == PersonalShopMessageCode::Available; }
};

enum class PersonalShopTradeCode
{
    Committed,
    InvalidRequest,
    SellerNotFound,
    BuyerNotFound,
    ListingNotFound,
    InsufficientGold,
    InsufficientCoin,
    SellerCoinCapacityExceeded,
    SellerWalletCapacityExceeded,
    CoinWalletRejected,
    PotionTransferRejected,
    ItemDeleteRejected,
    BuyerInventoryRejected,
    InventoryReservationRejected,
    WalletRejected,
    RollbackFailed
};

struct PersonalShopTradeRequest
{
    PlayerId seller;
    PlayerId buyer;
    std::uint32_t itemCode;
    std::uint32_t head;
    std::uint32_t checksum;
    std::uint32_t quantity;
    std::int64_t goldPrice;
    std::int64_t coinPrice;

    PersonalShopTradeRequest()
        : seller(), buyer(), itemCode(0u), head(0u), checksum(0u), quantity(0u), goldPrice(0), coinPrice(0) {}
};

struct PersonalShopTradeResult
{
    PersonalShopTradeCode code;
    PersonalShopTradeRequest request;

    PersonalShopTradeResult() : code(PersonalShopTradeCode::InvalidRequest), request() {}
    bool Committed() const { return code == PersonalShopTradeCode::Committed; }
};

class IPersonalShopService
{
public:
    virtual ~IPersonalShopService() {}
    virtual PersonalShopIngressResult HandleIngress(const PersonalShopIngressRequest& request) = 0;
    virtual PersonalShopPrepareResult PrepareTrade(const PersonalShopPrepareRequest& request) = 0;
    virtual PersonalShopMessageResult ReadListingMessage(const PersonalShopMessageRequest& request) = 0;
    virtual PersonalShopTradeResult CommitTrade(const PersonalShopTradeRequest& request) = 0;
};
}
}
