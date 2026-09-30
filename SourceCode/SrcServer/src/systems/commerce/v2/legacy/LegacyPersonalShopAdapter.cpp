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
#include "systems/gameplay/inventory/v2/legacy/LegacyPotionStackAdapter.h"

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
    IWalletCommands& walletCommands,
    const inventory::IInventoryQuery& inventoryQuery,
    inventory::IInventoryCommands& inventoryCommands,
    const ICoinWalletQuery& coinQuery,
    ICoinWalletCommands& coinCommands,
    const inventory::IPotionStackQuery& potionStackQuery,
    inventory::IPotionStackCommands& potionStackCommands)
    : registry_(registry), walletQuery_(walletQuery), walletCommands_(walletCommands),
      inventoryQuery_(inventoryQuery), inventoryCommands_(inventoryCommands),
      coinQuery_(coinQuery), coinCommands_(coinCommands),
      potionStackQuery_(potionStackQuery), potionStackCommands_(potionStackCommands)
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

    if(buyer->dwMyShopTradeTime > dwPlayServTime)
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

    buyer->dwMyShopTradeTime = dwPlayServTime + 500;
    if(buyer->MyShopListCount == 0)
    {
        ZeroMemory(buyer->sMyShopList, sizeof(sMYSHOP_SELL_INFO) * MYSHOP_LIST_MAX);
    }
    else
    {
        for(int cnt = 0; cnt < MYSHOP_LIST_MAX; ++cnt)
        {
            if(buyer->sMyShopList[cnt].dwCode &&
               request.itemCode == buyer->sMyShopList[cnt].dwCode &&
               request.head == buyer->sMyShopList[cnt].dwHead &&
               request.checksum == buyer->sMyShopList[cnt].dwChkSum &&
               request.goldPrice == static_cast<std::int64_t>(buyer->sMyShopList[cnt].Price))
            {
                result.code = PersonalShopPrepareCode::Prepared;
                return result;
            }
        }
    }

    const int slot = buyer->MyShopListCount & MYSHOP_LIST_MASK;
    buyer->sMyShopList[slot].dwCode = request.itemCode;
    buyer->sMyShopList[slot].dwHead = request.head;
    buyer->sMyShopList[slot].dwChkSum = request.checksum;
    buyer->sMyShopList[slot].Price = static_cast<int>(request.goldPrice);
    buyer->MyShopListCount++;

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

    WalletSnapshot buyerWallet;
    WalletSnapshot sellerWallet;
    if(!walletQuery_.TryGetWallet(request.buyer, buyerWallet) || buyerWallet.gold < request.goldPrice)
    {
        result.code = PersonalShopTradeCode::InsufficientGold;
        return result;
    }
    if(!walletQuery_.TryGetWallet(request.seller, sellerWallet) ||
       sellerWallet.gold < 0 || sellerWallet.gold > kLegacyWalletMax - request.goldPrice)
    {
        result.code = PersonalShopTradeCode::SellerWalletCapacityExceeded;
        return result;
    }
    CoinWalletSnapshot buyerCoinWallet;
    if(!coinQuery_.TryGetCoinWallet(request.buyer, buyerCoinWallet) ||
       buyerCoinWallet.coins < request.coinPrice)
    {
        result.code = PersonalShopTradeCode::InsufficientCoin;
        return result;
    }
    CoinWalletSnapshot sellerCoinWallet;
    if(!coinQuery_.TryGetCoinWallet(request.seller, sellerCoinWallet) ||
       sellerCoinWallet.coins < 0 ||
       sellerCoinWallet.coins > 0x7fffffffLL - request.coinPrice)
    {
        result.code = PersonalShopTradeCode::SellerCoinCapacityExceeded;
        return result;
    }

    const inventory::ItemInstanceId item(ItemId(request.itemCode), request.head, request.checksum);
    if(!inventoryQuery_.Contains(request.seller, item).Present())
    {
        result.code = PersonalShopTradeCode::ListingNotFound;
        return result;
    }

    const bool potion = (request.itemCode & sinITEM_MASK1) == (sinPM1 & sinITEM_MASK1);
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
            const inventory::PotionStackTransferResult potionRollback =
                potionStackCommands_.TransferStack(inventory::PotionStackTransferRequest(
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

    // Non-potion transfers reserve buyer capacity first. This closes the old
    // item-loss path where seller/gold mutation completed before buyer add was checked.
    if(!potion)
    {
        const inventory::InventoryMutationResult buyerAdd = inventoryCommands_.Add(request.buyer, item);
        if(buyerAdd.code != inventory::InventoryMutationCode::Applied)
        {
            result.code = PersonalShopTradeCode::BuyerInventoryRejected;
            return result;
        }
        buyerItemAdded = true;
    }
    else
    {
        const inventory::PotionStackTransferResult potionTransfer =
            potionStackCommands_.TransferStack(inventory::PotionStackTransferRequest(
                request.seller, request.buyer, ItemId(request.itemCode),
                static_cast<inventory::PotionQuantity>(request.quantity)));
        if(!potionTransfer.Applied())
        {
            rsServerPotionErrorLog(seller, request.itemCode, -static_cast<int>(request.quantity), 310);
            result.code = potionTransfer.code == inventory::PotionStackTransferCode::RollbackFailed
                              ? PersonalShopTradeCode::RollbackFailed
                              : PersonalShopTradeCode::PotionTransferRejected;
            return result;
        }
        potionMoved = true;
    }

    const inventory::InventoryMutationResult sellerRemove = inventoryCommands_.Remove(request.seller, item);
    if(sellerRemove.code != inventory::InventoryMutationCode::Applied)
    {
        const bool rollbackOk = rollbackInventory();
        result.code = rollbackOk ? PersonalShopTradeCode::ItemDeleteRejected
                                 : PersonalShopTradeCode::RollbackFailed;
        return result;
    }
    sellerItemRemoved = true;

    if(request.goldPrice > 0)
    {
        const WalletMutationResult debit =
            walletCommands_.Debit(request.buyer, request.goldPrice, WalletReason::PersonalShop);
        if(!debit.Ok())
        {
            const bool rollbackOk = rollbackInventory();
            result.code = rollbackOk ? PersonalShopTradeCode::WalletRejected
                                     : PersonalShopTradeCode::RollbackFailed;
            return result;
        }
        buyerGoldDebited = true;

        const WalletMutationResult credit =
            walletCommands_.Credit(request.seller, request.goldPrice, WalletReason::PersonalShop);
        if(!credit.Ok())
        {
            const bool goldRollbackOk = rollbackGold();
            const bool inventoryRollbackOk = rollbackInventory();
            const bool rollbackOk = goldRollbackOk && inventoryRollbackOk;
            result.code = rollbackOk ? PersonalShopTradeCode::WalletRejected
                                     : PersonalShopTradeCode::RollbackFailed;
            return result;
        }
        sellerGoldCredited = true;
    }

    if(request.coinPrice > 0)
    {
        const CoinMutationResult debit =
            coinCommands_.DebitCoins(request.buyer, request.coinPrice, CoinReason::PersonalShop);
        if(!debit.Ok())
        {
            const bool goldRollbackOk = rollbackGold();
            const bool inventoryRollbackOk = rollbackInventory();
            result.code = goldRollbackOk && inventoryRollbackOk
                              ? PersonalShopTradeCode::CoinWalletRejected
                              : PersonalShopTradeCode::RollbackFailed;
            return result;
        }
        buyerCoinDebited = true;

        const CoinMutationResult credit =
            coinCommands_.CreditCoins(request.seller, request.coinPrice, CoinReason::PersonalShop);
        if(!credit.Ok())
        {
            const bool coinRollbackOk = rollbackCoin();
            const bool goldRollbackOk = rollbackGold();
            const bool inventoryRollbackOk = rollbackInventory();
            result.code = coinRollbackOk && goldRollbackOk && inventoryRollbackOk
                              ? PersonalShopTradeCode::CoinWalletRejected
                              : PersonalShopTradeCode::RollbackFailed;
            return result;
        }
        sellerCoinCredited = true;
    }

    if(potion)
    {
        // Exact-item inventory metadata is intentionally a no-op for legacy potion stacks.
        const inventory::InventoryMutationResult buyerAdd = inventoryCommands_.Add(request.buyer, item);
        if(!buyerAdd.Applied())
        {
            const bool coinRollbackOk = rollbackCoin();
            const bool goldRollbackOk = rollbackGold();
            const bool inventoryRollbackOk = rollbackInventory();
            const bool rollbackOk = coinRollbackOk && goldRollbackOk && inventoryRollbackOk;
            result.code = rollbackOk ? PersonalShopTradeCode::BuyerInventoryRejected
                                     : PersonalShopTradeCode::RollbackFailed;
            return result;
        }
    }

    if(potion)
        rsRecord_ItemLog_MyShop(seller, buyer, request.itemCode, request.quantity, 0, static_cast<int>(request.goldPrice));
    else
        rsRecord_ItemLog_MyShop(seller, buyer, request.itemCode, request.head, request.checksum, static_cast<int>(request.goldPrice));

    result.code = PersonalShopTradeCode::Committed;
    return result;
}

LegacyPersonalShopAdapter& ServerLegacyPersonalShopAdapter()
{
    LegacyWalletAdapter& wallet = ServerLegacyWalletAdapter();
    inventory::legacy::LegacyInventoryRuntime& inventoryRuntime =
        inventory::legacy::ServerLegacyInventoryRuntime();
    LegacyCoinWalletAdapter& coin = ServerLegacyCoinWalletAdapter();
    inventory::legacy::LegacyPotionStackAdapter& potionStacks =
        inventory::legacy::ServerLegacyPotionStackAdapter();
    static LegacyPersonalShopAdapter adapter(
        player::legacy::ServerLegacyOnlinePlayerRegistry(),
        wallet,
        wallet,
        inventoryRuntime,
        inventoryRuntime,
        coin,
        coin,
        potionStacks,
        potionStacks);
    return adapter;
}
}
}
}
