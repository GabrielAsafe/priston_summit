#include "gameplay/commerce/v2/PersonalShopService.h"

#include <cassert>
#include <iostream>

namespace
{
class FakePersonalShopService final : public gameplay::commerce::IPersonalShopService
{
public:
    gameplay::commerce::PersonalShopIngressResult HandleIngress(
        const gameplay::commerce::PersonalShopIngressRequest& request) override
    {
        gameplay::commerce::PersonalShopIngressResult result;
        result.sender = request.sender;
        result.action = request.action;
        result.receiver = gameplay::PlayerHandle(request.receiver, 7u);
        result.code = request.sender.Valid()
                          ? gameplay::commerce::PersonalShopIngressCode::Allowed
                          : gameplay::commerce::PersonalShopIngressCode::SenderNotFound;
        return result;
    }

    gameplay::commerce::PersonalShopPrepareResult PrepareTrade(
        const gameplay::commerce::PersonalShopPrepareRequest& request) override
    {
        gameplay::commerce::PersonalShopPrepareResult result;
        result.request = request;
        result.code = request.buyer.Valid() && request.seller.Valid()
                          ? gameplay::commerce::PersonalShopPrepareCode::Prepared
                          : gameplay::commerce::PersonalShopPrepareCode::InvalidRequest;
        return result;
    }

    gameplay::commerce::PersonalShopMessageResult ReadListingMessage(
        const gameplay::commerce::PersonalShopMessageRequest& request) override
    {
        gameplay::commerce::PersonalShopMessageResult result;
        result.target = request.target;
        if(request.viewer.Valid() && request.target.Valid() && request.listingCode == 77u)
        {
            result.code = gameplay::commerce::PersonalShopMessageCode::Available;
            result.message = "shop";
        }
        else
        {
            result.code = gameplay::commerce::PersonalShopMessageCode::ListingCodeMismatch;
        }
        return result;
    }

    gameplay::commerce::PersonalShopTradeResult CommitTrade(
        const gameplay::commerce::PersonalShopTradeRequest& request) override
    {
        gameplay::commerce::PersonalShopTradeResult result;
        result.request = request;
        result.code = request.seller.Valid() && request.buyer.Valid()
                          ? gameplay::commerce::PersonalShopTradeCode::Committed
                          : gameplay::commerce::PersonalShopTradeCode::InvalidRequest;
        return result;
    }
};
}

int main()
{
    using namespace gameplay;
    using namespace gameplay::commerce;

    FakePersonalShopService service;

    PersonalShopIngressRequest open(PlayerId(11u), PlayerId(), PersonalShopIngressAction::OpenListing);
    open.listingMessage = "shop open";
    open.listingCode = 33u;
    const PersonalShopIngressResult opened = service.HandleIngress(open);
    assert(opened.Allowed());
    assert(opened.sender == PlayerId(11u));

    PersonalShopIngressRequest trade(PlayerId(11u), PlayerId(22u), PersonalShopIngressAction::TradeRequest);
    const PersonalShopIngressResult relayed = service.HandleIngress(trade);
    assert(relayed.Allowed());
    assert(relayed.receiver.id == PlayerId(22u));
    assert(relayed.receiver.generation == 7u);

    const PersonalShopIngressResult rejected = service.HandleIngress(PersonalShopIngressRequest());
    assert(!rejected.Allowed());
    assert(rejected.code == PersonalShopIngressCode::SenderNotFound);

    PersonalShopTradeRequest purchase;
    purchase.seller = PlayerId(11u);
    purchase.buyer = PlayerId(22u);
    purchase.itemCode = 0x1234u;
    purchase.goldPrice = 500;
    const PersonalShopTradeResult committed = service.CommitTrade(purchase);
    assert(committed.Committed());
    assert(committed.request.goldPrice == 500);

    PersonalShopPrepareRequest prepare;
    prepare.buyer = PlayerId(22u);
    prepare.seller = PlayerId(11u);
    prepare.itemCode = 0x1234u;
    const PersonalShopPrepareResult prepared = service.PrepareTrade(prepare);
    assert(prepared.Prepared());

    const PersonalShopMessageResult message = service.ReadListingMessage(
        PersonalShopMessageRequest(PlayerId(22u), PlayerId(11u), 77u));
    assert(message.Available());
    assert(message.message == "shop");

    std::cout << "personal shop session + trade smoke: PASS\n";
    return 0;
}
