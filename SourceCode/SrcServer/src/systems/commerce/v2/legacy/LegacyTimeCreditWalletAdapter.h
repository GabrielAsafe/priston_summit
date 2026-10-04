#pragma once

#include <limits>

#include "gameplay/commerce/v2/CoinWallet.h"
#include "systems/commerce/coin_shop/CoinShop_T.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

namespace gameplay
{
namespace commerce
{
namespace legacy
{
// AMBER bridge for the legacy UserInfo_T/UserMinutos balance.  The GREEN
// transaction service is intentionally denomination-agnostic even though the
// historic wallet contract is named CoinWallet.
class LegacyTimeCreditWalletAdapter final : public ICoinWalletQuery, public ICoinWalletCommands
{
public:
    explicit LegacyTimeCreditWalletAdapter(
        const player::legacy::LegacyOnlinePlayerRegistry& registry)
        : registry_(registry) {}

    bool TryGetCoinWallet(PlayerId player, CoinWalletSnapshot& out) const override
    {
        rsPLAYINFO* legacyPlayer = registry_.Resolve(player);
        if(!legacyPlayer)
            return false;

        const int balance = cCoinShop_T.GetPlayerCoin(legacyPlayer);
        if(balance < 0)
            return false;

        out = CoinWalletSnapshot(player, static_cast<CoinAmount>(balance));
        return true;
    }

    CoinMutationResult CreditCoins(PlayerId player,
                                   CoinAmount amount,
                                   CoinReason /*reason*/) override
    {
        rsPLAYINFO* legacyPlayer = registry_.Resolve(player);
        if(!legacyPlayer)
            return MissingPlayer(player, amount);

        const CoinAmount before = static_cast<CoinAmount>(cCoinShop_T.GetPlayerCoin(legacyPlayer));
        CoinMutationResult result = BaseResult(player, amount, before);
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

        cCoinShop_T.AddPlayerCoin(legacyPlayer, static_cast<int>(amount));
        result.after = static_cast<CoinAmount>(cCoinShop_T.GetPlayerCoin(legacyPlayer));
        result.code = result.after == before + amount
                          ? CoinMutationCode::Ok
                          : CoinMutationCode::LegacyRejected;
        return result;
    }

    CoinMutationResult DebitCoins(PlayerId player,
                                  CoinAmount amount,
                                  CoinReason /*reason*/) override
    {
        rsPLAYINFO* legacyPlayer = registry_.Resolve(player);
        if(!legacyPlayer)
            return MissingPlayer(player, amount);

        const CoinAmount before = static_cast<CoinAmount>(cCoinShop_T.GetPlayerCoin(legacyPlayer));
        CoinMutationResult result = BaseResult(player, amount, before);
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

        cCoinShop_T.AddPlayerCoin(legacyPlayer, -static_cast<int>(amount));
        result.after = static_cast<CoinAmount>(cCoinShop_T.GetPlayerCoin(legacyPlayer));
        result.code = result.after == before - amount
                          ? CoinMutationCode::Ok
                          : CoinMutationCode::LegacyRejected;
        return result;
    }

private:
    static CoinMutationResult BaseResult(PlayerId player, CoinAmount amount, CoinAmount before)
    {
        CoinMutationResult result;
        result.player = player;
        result.requested = amount;
        result.before = before;
        result.after = before;
        return result;
    }

    static CoinMutationResult MissingPlayer(PlayerId player, CoinAmount amount)
    {
        CoinMutationResult result;
        result.player = player;
        result.requested = amount;
        result.code = CoinMutationCode::PlayerNotFound;
        return result;
    }

    const player::legacy::LegacyOnlinePlayerRegistry& registry_;
};

inline LegacyTimeCreditWalletAdapter& ServerLegacyTimeCreditWalletAdapter()
{
    static LegacyTimeCreditWalletAdapter adapter(player::legacy::ServerLegacyOnlinePlayerRegistry());
    return adapter;
}
}
}
}
