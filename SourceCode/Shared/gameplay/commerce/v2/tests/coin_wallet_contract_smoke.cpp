#include "gameplay/commerce/v2/CoinWallet.h"

#include <cassert>
#include <iostream>
#include <map>

namespace
{
class FakeCoinWallet final : public gameplay::commerce::ICoinWalletQuery,
                             public gameplay::commerce::ICoinWalletCommands
{
public:
    bool TryGetCoinWallet(gameplay::PlayerId player, gameplay::commerce::CoinWalletSnapshot& out) const override
    {
        const auto it = balances.find(player.value);
        if(!player.Valid() || it == balances.end())
            return false;
        out = gameplay::commerce::CoinWalletSnapshot(player, it->second);
        return true;
    }

    gameplay::commerce::CoinMutationResult CreditCoins(
        gameplay::PlayerId player,
        gameplay::commerce::CoinAmount amount,
        gameplay::commerce::CoinReason) override
    {
        return Mutate(player, amount, false);
    }

    gameplay::commerce::CoinMutationResult DebitCoins(
        gameplay::PlayerId player,
        gameplay::commerce::CoinAmount amount,
        gameplay::commerce::CoinReason) override
    {
        return Mutate(player, amount, true);
    }

    std::map<std::uint32_t, gameplay::commerce::CoinAmount> balances;

private:
    gameplay::commerce::CoinMutationResult Mutate(
        gameplay::PlayerId player,
        gameplay::commerce::CoinAmount amount,
        bool debit)
    {
        gameplay::commerce::CoinMutationResult result;
        result.player = player;
        result.requested = amount;
        if(amount <= 0)
        {
            result.code = gameplay::commerce::CoinMutationCode::InvalidAmount;
            return result;
        }
        const auto it = balances.find(player.value);
        if(!player.Valid() || it == balances.end())
        {
            result.code = gameplay::commerce::CoinMutationCode::PlayerNotFound;
            return result;
        }
        result.before = it->second;
        result.after = it->second;
        if(debit && it->second < amount)
        {
            result.code = gameplay::commerce::CoinMutationCode::InsufficientFunds;
            return result;
        }
        it->second += debit ? -amount : amount;
        result.after = it->second;
        result.code = gameplay::commerce::CoinMutationCode::Ok;
        return result;
    }
};
}

int main()
{
    using namespace gameplay;
    using namespace gameplay::commerce;
    FakeCoinWallet wallet;
    wallet.balances[1u] = 100;
    CoinWalletSnapshot snap;
    assert(wallet.TryGetCoinWallet(PlayerId(1u), snap));
    assert(snap.coins == 100);
    assert(wallet.DebitCoins(PlayerId(1u), 40, CoinReason::Trade).Ok());
    assert(wallet.CreditCoins(PlayerId(1u), 15, CoinReason::Trade).Ok());
    assert(wallet.DebitCoins(PlayerId(1u), 1000, CoinReason::Trade).code == CoinMutationCode::InsufficientFunds);
    std::cout << "coin wallet contract smoke: PASS\n";
    return 0;
}
