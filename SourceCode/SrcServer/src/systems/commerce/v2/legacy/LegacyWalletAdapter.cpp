#include "systems/commerce/v2/legacy/LegacyWalletAdapter.h"

#include <limits>

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"

namespace gameplay
{
namespace commerce
{
namespace legacy
{
namespace
{
const CurrencyAmount kLegacyWalletMax = 1000000000LL;

int LegacyWhere(WalletReason reason)
{
    switch(reason)
    {
        case WalletReason::Trade: return WHERE_TRADE;
        case WalletReason::GiftExpress: return WHERE_GIFT_EXPRESS;
        case WalletReason::Aging: return WHERE_AGINGITEM;
        case WalletReason::Crafting: return WHERE_CRAFTITEM;
        case WalletReason::ForceOrb: return WHERE_FORECORB;
        case WalletReason::LearnSkill: return WHERE_LEARNSKILL;
        case WalletReason::WarpGate: return WHERE_WARPGATE;
        case WalletReason::NpcBuyItem: return WHERE_BUY_ITEM;
        case WalletReason::NpcBuyPotion: return WHERE_BUY_POTION;
        case WalletReason::NpcSellItem: return WHERE_SELL_ITEM;
        case WalletReason::ThrowItem: return WHERE_THROW_ITEM;
        case WalletReason::PickupItem: return WHERE_GET_ITEM;
        case WalletReason::StarPoint: return WHERE_STAR_POINT;
        case WalletReason::ClanMoney: return WHERE_CLAN_MONEY;
        case WalletReason::GiveMoney: return WHERE_GIVE_MONEY;
        case WalletReason::PersonalShop: return WHERE_PERSONAL_SHOP;
        case WalletReason::Warehouse: return WHERE_OPEN_WAREHOUES;
    }
    return WHERE_GIVE_MONEY;
}

WalletMutationResult BaseResult(PlayerId player, CurrencyAmount amount, CurrencyAmount before)
{
    WalletMutationResult result;
    result.player = player;
    result.requested = amount;
    result.before = before;
    result.after = before;
    return result;
}
}

LegacyWalletAdapter::LegacyWalletAdapter(const player::legacy::LegacyOnlinePlayerRegistry& registry)
    : registry_(registry)
{
}

bool LegacyWalletAdapter::TryGetWallet(PlayerId player, WalletSnapshot& out) const
{
    rsPLAYINFO* legacyPlayer = registry_.Resolve(player);
    if(!legacyPlayer)
        return false;

    out = WalletSnapshot(player, static_cast<CurrencyAmount>(legacyPlayer->ServerMoney));
    return true;
}

WalletMutationResult LegacyWalletAdapter::Credit(PlayerId player, CurrencyAmount amount, WalletReason reason)
{
    rsPLAYINFO* legacyPlayer = registry_.Resolve(player);
    if(!legacyPlayer)
    {
        WalletMutationResult result;
        result.player = player;
        result.requested = amount;
        result.code = WalletMutationCode::PlayerNotFound;
        return result;
    }

    const CurrencyAmount before = static_cast<CurrencyAmount>(legacyPlayer->ServerMoney);
    WalletMutationResult result = BaseResult(player, amount, before);
    if(amount <= 0 || amount > static_cast<CurrencyAmount>(std::numeric_limits<int>::max()))
    {
        result.code = WalletMutationCode::InvalidAmount;
        return result;
    }
    if(before < 0 || before > kLegacyWalletMax || amount > kLegacyWalletMax - before)
    {
        result.code = WalletMutationCode::CapacityExceeded;
        return result;
    }

    const int accepted = legacyPlayer->AddServerMoney(static_cast<int>(amount), LegacyWhere(reason));
    result.after = static_cast<CurrencyAmount>(legacyPlayer->ServerMoney);
    result.code = accepted && result.after == before + amount
                      ? WalletMutationCode::Ok
                      : WalletMutationCode::LegacyRejected;
    return result;
}

WalletMutationResult LegacyWalletAdapter::Debit(PlayerId player, CurrencyAmount amount, WalletReason reason)
{
    rsPLAYINFO* legacyPlayer = registry_.Resolve(player);
    if(!legacyPlayer)
    {
        WalletMutationResult result;
        result.player = player;
        result.requested = amount;
        result.code = WalletMutationCode::PlayerNotFound;
        return result;
    }

    const CurrencyAmount before = static_cast<CurrencyAmount>(legacyPlayer->ServerMoney);
    WalletMutationResult result = BaseResult(player, amount, before);
    if(amount <= 0 || amount > static_cast<CurrencyAmount>(std::numeric_limits<int>::max()) || amount > kLegacyWalletMax)
    {
        result.code = WalletMutationCode::InvalidAmount;
        return result;
    }
    if(before < amount)
    {
        result.code = WalletMutationCode::InsufficientFunds;
        return result;
    }

    const int accepted = legacyPlayer->SubServerMoney(static_cast<int>(amount), LegacyWhere(reason));
    result.after = static_cast<CurrencyAmount>(legacyPlayer->ServerMoney);
    result.code = accepted && result.after == before - amount
                      ? WalletMutationCode::Ok
                      : WalletMutationCode::LegacyRejected;
    return result;
}

LegacyWalletAdapter& ServerLegacyWalletAdapter()
{
    static LegacyWalletAdapter adapter(player::legacy::ServerLegacyOnlinePlayerRegistry());
    return adapter;
}
}
}
}
