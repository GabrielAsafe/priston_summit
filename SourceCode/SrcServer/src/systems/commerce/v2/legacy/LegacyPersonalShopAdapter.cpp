#include "systems/commerce/v2/legacy/LegacyPersonalShopAdapter.h"

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"
#include "core/server/gameplay_runtime/playmain.h"
#include "systems/commerce/v2/legacy/LegacyWalletAdapter.h"
#include "systems/commerce/v2/legacy/LegacyCoinWalletAdapter.h"
#include "systems/inventory/legacy/sinItem.h"
#include "systems/inventory/v2/legacy/LegacyInventoryRuntime.h"
#include "systems/inventory/v2/legacy/LegacyInventoryReservationRuntime.h"
#include "systems/gameplay/inventory/v2/legacy/LegacyPotionStackAdapter.h"
#include "gameplay/commerce/v2/PersonalShopSessionService.h"
#include "gameplay/commerce/v2/PersonalShopTransactionService.h"

int rsServerPotionErrorLog(rsPLAYINFO* lpPlayInfo, DWORD dwCode, int Count, int Position);
int rsRecord_ItemLog_MyShop(rsPLAYINFO* lpPlayInfo, rsPLAYINFO* lpPlayInfo2, DWORD dwItemCode, DWORD dwHead, DWORD dwChkSum, int Price);

namespace gameplay
{
namespace commerce
{
namespace legacy
{
namespace
{
bool RequiresTradingEnabled(PersonalShopIngressAction action)
{
    return action != PersonalShopIngressAction::RelayItem;
}

bool RequiresReceiver(PersonalShopIngressAction action)
{
    return action != PersonalShopIngressAction::OpenListing;
}
}

LegacyPersonalShopAdapter::LegacyPersonalShopAdapter(
    const player::legacy::LegacyOnlinePlayerRegistry& registry,
    const IWalletQuery& walletQuery,
    const inventory::IInventoryQuery& inventoryQuery,
    IPersonalShopSessionService& sessionService,
    const ICoinWalletQuery& coinQuery,
    const inventory::IPotionStackQuery& potionStackQuery,
    IPersonalShopTransactionService& transactionService)
    : registry_(registry), walletQuery_(walletQuery), inventoryQuery_(inventoryQuery),
      sessionService_(sessionService), coinQuery_(coinQuery),
      potionStackQuery_(potionStackQuery), transactionService_(transactionService)
{
}

PersonalShopIngressResult LegacyPersonalShopAdapter::HandleIngress(
    const PersonalShopIngressRequest& request)
{
    PersonalShopIngressResult result;
    result.sender = request.sender;
    result.action = request.action;

    rsPLAYINFO* sender = registry_.Resolve(request.sender);
    if(!sender)
    {
        result.code = PersonalShopIngressCode::SenderNotFound;
        return result;
    }

    if(RequiresTradingEnabled(request.action))
    {
        if(sender->szServerID[0])
        {
            result.code = PersonalShopIngressCode::ServerModeBlocked;
            return result;
        }
        if(rsServerConfig.DisableThrowItem)
        {
            result.code = PersonalShopIngressCode::TradingDisabled;
            return result;
        }
    }

    if(request.action == PersonalShopIngressAction::OpenListing)
    {
        strcpy_s(sender->szTradeMessage, request.listingMessage.c_str());
        sender->dwTradeMsgCode = static_cast<DWORD>(request.listingCode);
        result.code = PersonalShopIngressCode::Allowed;
        return result;
    }

    if(RequiresReceiver(request.action))
    {
        rsPLAYINFO* receiver = registry_.Resolve(request.receiver);
        if(!receiver || !receiver->lpsmSock)
        {
            result.code = PersonalShopIngressCode::ReceiverNotFound;
            return result;
        }
        result.receiver = registry_.HandleOf(receiver);
    }

    result.code = PersonalShopIngressCode::Allowed;
    return result;
}


PersonalShopPrepareResult LegacyPersonalShopAdapter::PrepareTrade(const PersonalShopPrepareRequest& request)
{
    static const CurrencyAmount kLegacyWalletMax = 1000000000LL;
    PersonalShopPrepareResult result;
    result.request = request;

    if(!request.buyer.Valid() || !request.seller.Valid() || request.buyer == request.seller ||
       request.itemCode == 0u || request.goldPrice < 0 || request.goldPrice > kLegacyWalletMax ||
       request.coinPrice < 0 || request.coinPrice > 0x7fffffffLL)
    {
        result.code = PersonalShopPrepareCode::InvalidRequest;
        return result;
    }

    rsPLAYINFO* buyer = registry_.Resolve(request.buyer);
    if(!buyer)
    {
        result.code = PersonalShopPrepareCode::BuyerNotFound;
        return result;
    }
    rsPLAYINFO* seller = registry_.Resolve(request.seller);
    if(!seller)
    {
        result.code = PersonalShopPrepareCode::SellerNotFound;
        return result;
    }

    const PlayerHandle buyerHandle = registry_.HandleOf(buyer);
    if(sessionService_.CooldownActive(buyerHandle, static_cast<std::uint32_t>(dwPlayServTime)))
    {
        result.code = PersonalShopPrepareCode::CooldownActive;
        return result;
    }

    WalletSnapshot buyerWallet;
    CoinWalletSnapshot buyerCoinWallet;
    const bool hasGold = walletQuery_.TryGetWallet(request.buyer, buyerWallet) && buyerWallet.gold >= request.goldPrice;
    const bool hasCoin = coinQuery_.TryGetCoinWallet(request.buyer, buyerCoinWallet) &&
                         buyerCoinWallet.coins >= request.coinPrice;
    if(!hasGold || !hasCoin)
    {
        result.code = PersonalShopPrepareCode::InsufficientCurrency;
        return result;
    }

    bool potionAvailable = true;
    if((request.itemCode & sinITEM_MASK1) == (sinPM1 & sinITEM_MASK1))
    {
        inventory::PotionStackSnapshot potionStack;
        potionAvailable = potionStackQuery_.TryGetStack(
                              request.seller, ItemId(request.itemCode), potionStack) &&
                          potionStack.quantity >= static_cast<inventory::PotionQuantity>(request.quantity);
    }

    const inventory::ItemInstanceId item(
        ItemId(request.itemCode), request.head, request.checksum);
    const bool itemAvailable =
        inventoryQuery_.Contains(request.seller, item).Present() && potionAvailable;
    if(!itemAvailable)
    {
        result.code = PersonalShopPrepareCode::ItemUnavailable;
        result.sendUnavailableReply = true;
        return result;
    }

    const PersonalShopSessionRecordResult session = sessionService_.RecordPrepared(
        buyerHandle,
        static_cast<std::uint32_t>(dwPlayServTime),
        PersonalShopSessionFingerprint(request.itemCode, request.head, request.checksum, request.goldPrice));
    if(!session.Accepted())
    {
        result.code = PersonalShopPrepareCode::InvalidRequest;
        return result;
    }

    result.code = PersonalShopPrepareCode::Prepared;
    return result;
}

PersonalShopMessageResult LegacyPersonalShopAdapter::ReadListingMessage(const PersonalShopMessageRequest& request)
{
    PersonalShopMessageResult result;
    result.target = request.target;

    rsPLAYINFO* viewer = registry_.Resolve(request.viewer);
    if(!viewer)
    {
        result.code = PersonalShopMessageCode::ViewerNotFound;
        return result;
    }
    if(viewer->szServerID[0])
    {
        result.code = PersonalShopMessageCode::ServerModeBlocked;
        return result;
    }
    if(rsServerConfig.DisableThrowItem)
    {
        result.code = PersonalShopMessageCode::TradingDisabled;
        return result;
    }

    rsPLAYINFO* target = registry_.Resolve(request.target);
    if(!target)
    {
        result.code = PersonalShopMessageCode::TargetNotFound;
        return result;
    }
    if(!target->dwTradeMsgCode || target->dwTradeMsgCode != static_cast<DWORD>(request.listingCode))
    {
        result.code = PersonalShopMessageCode::ListingCodeMismatch;
        return result;
    }

    result.message = target->szTradeMessage;
    result.code = PersonalShopMessageCode::Available;
    return result;
}


PersonalShopTradeResult LegacyPersonalShopAdapter::CommitTrade(const PersonalShopTradeRequest& request)
{
    static const CurrencyAmount kLegacyWalletMax = 1000000000LL;
    PersonalShopTradeResult result;
    result.request = request;

    if(!request.seller.Valid() || !request.buyer.Valid() || request.seller == request.buyer ||
       request.itemCode == 0u || request.goldPrice < 0 || request.goldPrice > kLegacyWalletMax ||
       request.coinPrice < 0 || request.coinPrice > 0x7fffffffLL ||
       (request.goldPrice == 0 && request.coinPrice == 0))
    {
        result.code = PersonalShopTradeCode::InvalidRequest;
        return result;
    }

    rsPLAYINFO* seller = registry_.Resolve(request.seller);
    if(!seller)
    {
        result.code = PersonalShopTradeCode::SellerNotFound;
        return result;
    }
    rsPLAYINFO* buyer = registry_.Resolve(request.buyer);
    if(!buyer)
    {
        result.code = PersonalShopTradeCode::BuyerNotFound;
        return result;
    }

    bool listed = false;
    for(int cnt = 0; cnt < MYSHOP_LIST_MAX; ++cnt)
    {
        if(seller->sMyShopList[cnt].dwCode &&
           request.itemCode == seller->sMyShopList[cnt].dwCode &&
           request.head == seller->sMyShopList[cnt].dwHead &&
           request.checksum == seller->sMyShopList[cnt].dwChkSum &&
           request.goldPrice == static_cast<std::int64_t>(seller->sMyShopList[cnt].Price))
        {
            listed = true;
            break;
        }
    }
    if(!listed)
    {
        result.code = PersonalShopTradeCode::ListingNotFound;
        return result;
    }

    const bool potion = (request.itemCode & sinITEM_MASK1) == (sinPM1 & sinITEM_MASK1);
    const PersonalShopTransactionResult transaction = transactionService_.Commit(
        PersonalShopTransactionRequest(request, potion));

    if(transaction.potionTransferFailed)
        rsServerPotionErrorLog(seller, request.itemCode, -static_cast<int>(request.quantity), 310);

    if(transaction.trade.Committed())
    {
        if(potion)
            rsRecord_ItemLog_MyShop(seller, buyer, request.itemCode, request.quantity, 0,
                                    static_cast<int>(request.goldPrice));
        else
            rsRecord_ItemLog_MyShop(seller, buyer, request.itemCode, request.head, request.checksum,
                                    static_cast<int>(request.goldPrice));
    }

    return transaction.trade;
}

LegacyPersonalShopAdapter& ServerLegacyPersonalShopAdapter()
{
    LegacyWalletAdapter& wallet = ServerLegacyWalletAdapter();
    inventory::legacy::LegacyInventoryRuntime& inventoryRuntime =
        inventory::legacy::ServerLegacyInventoryRuntime();
    LegacyCoinWalletAdapter& coin = ServerLegacyCoinWalletAdapter();
    inventory::legacy::LegacyPotionStackAdapter& potionStacks =
        inventory::legacy::ServerLegacyPotionStackAdapter();
    static PersonalShopSessionService sessionService(MYSHOP_LIST_MAX, 500u);
    static PersonalShopTransactionService transactionService(
        wallet, wallet, inventoryRuntime, inventoryRuntime,
        inventory::legacy::ServerInventoryReservationService(), coin, coin, potionStacks);
    static LegacyPersonalShopAdapter adapter(
        player::legacy::ServerLegacyOnlinePlayerRegistry(),
        wallet,
        inventoryRuntime,
        sessionService,
        coin,
        potionStacks,
        transactionService);
    return adapter;
}
}
}
}
