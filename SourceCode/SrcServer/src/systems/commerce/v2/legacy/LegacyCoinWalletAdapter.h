#pragma once

#include "gameplay/commerce/v2/CoinWallet.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

namespace gameplay
{
namespace commerce
{
namespace legacy
{
class LegacyCoinWalletAdapter final : public ICoinWalletQuery, public ICoinWalletCommands
{
public:
    explicit LegacyCoinWalletAdapter(const player::legacy::LegacyOnlinePlayerRegistry& registry);

    bool TryGetCoinWallet(PlayerId player, CoinWalletSnapshot& out) const override;
    CoinMutationResult CreditCoins(PlayerId player, CoinAmount amount, CoinReason reason) override;
    CoinMutationResult DebitCoins(PlayerId player, CoinAmount amount, CoinReason reason) override;

private:
    const player::legacy::LegacyOnlinePlayerRegistry& registry_;
};

LegacyCoinWalletAdapter& ServerLegacyCoinWalletAdapter();
}
}
}
