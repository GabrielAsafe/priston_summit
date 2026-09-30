#include "systems/commerce/v2/legacy/LegacyCoinWalletAdapter.h"

#include <limits>

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"
#include "systems/commerce/coin_shop/CoinShop.h"

namespace gameplay
{
namespace commerce
{
namespace legacy
{
namespace
{
CoinMutationResult BaseCoinResult(PlayerId player, CoinAmount amount, CoinAmount before)
{
    CoinMutationResult result;
    result.player = player;
    result.requested = amount;
    result.before = before;
    result.after = before;
    return result;
}
}

LegacyCoinWalletAdapter::LegacyCoinWalletAdapter(
    const player::legacy::LegacyOnlinePlayerRegistry& registry)
    : registry_(registry)
{
}

bool LegacyCoinWalletAdapter::TryGetCoinWallet(PlayerId player, CoinWalletSnapshot& out) const
{
    rsPLAYINFO* legacyPlayer = registry_.Resolve(player);
    if(!legacyPlayer)
        return false;

    const int coins = cCoinShop.GetPlayerCoin(legacyPlayer);
    if(coins < 0)
        return false;

    out = CoinWalletSnapshot(player, static_cast<CoinAmount>(coins));
    return true;
}

CoinMutationResult LegacyCoinWalletAdapter::CreditCoins(
    PlayerId player,
    CoinAmount amount,
    CoinReason /*reason*/)
{
    rsPLAYINFO* legacyPlayer = registry_.Resolve(player);
    if(!legacyPlayer)
    {
        CoinMutationResult result;
        result.player = player;
        result.requested = amount;
        result.code = CoinMutationCode::PlayerNotFound;
        return result;
    }

    const CoinAmount before = static_cast<CoinAmount>(cCoinShop.GetPlayerCoin(legacyPlayer));
    CoinMutationResult result = BaseCoinResult(player, amount, before);
    if(amount <= 0 || amount > static_cast<CoinAmount>(std::numeric_limits<int>::max()))
    {
        result.code = CoinMutationCode::InvalidAmount;
        return result;
    }
    if(before < 0 || before > static_cast<CoinAmount>(std::numeric_limits<int>::max()) - amount)
    {
        result.code = CoinMutationCode::CapacityExceeded;
        return result;
    }

    cCoinShop.AddPlayerCoin(legacyPlayer, static_cast<int>(amount));
    result.after = static_cast<CoinAmount>(cCoinShop.GetPlayerCoin(legacyPlayer));
    result.code = result.after == before + amount
                      ? CoinMutationCode::Ok
                      : CoinMutationCode::LegacyRejected;
    return result;
}

CoinMutationResult LegacyCoinWalletAdapter::DebitCoins(
    PlayerId player,
    CoinAmount amount,
    CoinReason /*reason*/)
{
    rsPLAYINFO* legacyPlayer = registry_.Resolve(player);
    if(!legacyPlayer)
    {
        CoinMutationResult result;
        result.player = player;
        result.requested = amount;
        result.code = CoinMutationCode::PlayerNotFound;
        return result;
    }

    const CoinAmount before = static_cast<CoinAmount>(cCoinShop.GetPlayerCoin(legacyPlayer));
    CoinMutationResult result = BaseCoinResult(player, amount, before);
    if(amount <= 0 || amount > static_cast<CoinAmount>(std::numeric_limits<int>::max()))
    {
        result.code = CoinMutationCode::InvalidAmount;
        return result;
    }
    if(before < amount)
    {
        result.code = CoinMutationCode::InsufficientFunds;
        return result;
    }

    cCoinShop.AddPlayerCoin(legacyPlayer, -static_cast<int>(amount));
    result.after = static_cast<CoinAmount>(cCoinShop.GetPlayerCoin(legacyPlayer));
    result.code = result.after == before - amount
                      ? CoinMutationCode::Ok
                      : CoinMutationCode::LegacyRejected;
    return result;
}

LegacyCoinWalletAdapter& ServerLegacyCoinWalletAdapter()
{
    static LegacyCoinWalletAdapter adapter(player::legacy::ServerLegacyOnlinePlayerRegistry());
    return adapter;
}
}
}
}
