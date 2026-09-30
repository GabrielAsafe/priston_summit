#include <cassert>
#include <map>

#include "gameplay/commerce/v2/Wallet.h"

namespace
{
class FakeWallet final : public gameplay::commerce::IWalletQuery,
                         public gameplay::commerce::IWalletCommands
{
public:
    bool TryGetWallet(gameplay::PlayerId player, gameplay::commerce::WalletSnapshot& out) const override
    {
        std::map<std::uint32_t, gameplay::commerce::CurrencyAmount>::const_iterator it = balances.find(player.value);
        if(!player.Valid() || it == balances.end())
            return false;
        out = gameplay::commerce::WalletSnapshot(player, it->second);
        return true;
    }

    gameplay::commerce::WalletMutationResult Credit(
        gameplay::PlayerId player, gameplay::commerce::CurrencyAmount amount,
        gameplay::commerce::WalletReason) override
    {
        return Change(player, amount, false);
    }

    gameplay::commerce::WalletMutationResult Debit(
        gameplay::PlayerId player, gameplay::commerce::CurrencyAmount amount,
        gameplay::commerce::WalletReason) override
    {
        return Change(player, amount, true);
    }

    std::map<std::uint32_t, gameplay::commerce::CurrencyAmount> balances;

private:
    gameplay::commerce::WalletMutationResult Change(
        gameplay::PlayerId player, gameplay::commerce::CurrencyAmount amount, bool debit)
    {
        using namespace gameplay::commerce;
        WalletMutationResult result;
        result.player = player;
        result.requested = amount;
        if(!player.Valid() || balances.count(player.value) == 0u)
        {
            result.code = WalletMutationCode::PlayerNotFound;
            return result;
        }
        result.before = balances[player.value];
        result.after = result.before;
        if(amount <= 0)
        {
            result.code = WalletMutationCode::InvalidAmount;
            return result;
        }
        if(debit && result.before < amount)
        {
            result.code = WalletMutationCode::InsufficientFunds;
            return result;
        }
        result.after = debit ? result.before - amount : result.before + amount;
        balances[player.value] = result.after;
        result.code = WalletMutationCode::Ok;
        return result;
    }
};
}

int main()
{
    using namespace gameplay;
    using namespace gameplay::commerce;

    FakeWallet wallet;
    wallet.balances[7u] = 500;

    WalletSnapshot snapshot;
    assert(wallet.TryGetWallet(PlayerId(7u), snapshot));
    assert(snapshot.gold == 500);

    WalletMutationResult result = wallet.Debit(PlayerId(7u), 125, WalletReason::Trade);
    assert(result.Ok());
    assert(result.before == 500 && result.after == 375);

    result = wallet.Credit(PlayerId(7u), 25, WalletReason::NpcSellItem);
    assert(result.Ok());
    assert(result.after == 400);

    result = wallet.Debit(PlayerId(7u), 401, WalletReason::NpcBuyItem);
    assert(result.code == WalletMutationCode::InsufficientFunds);
    assert(wallet.balances[7u] == 400);

    result = wallet.Credit(PlayerId(7u), 0, WalletReason::Trade);
    assert(result.code == WalletMutationCode::InvalidAmount);
    return 0;
}
