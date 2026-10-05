#pragma once

#include "gameplay/commerce/v2/Wallet.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

namespace gameplay
{
namespace commerce
{
namespace legacy
{
class LegacyWalletAdapter final : public IWalletQuery, public IWalletCommands
{
public:
    explicit LegacyWalletAdapter(const player::legacy::LegacyOnlinePlayerRegistry& registry);

    bool TryGetWallet(PlayerId player, WalletSnapshot& out) const override;
    WalletMutationResult Credit(PlayerId player, CurrencyAmount amount, WalletReason reason) override;
    WalletMutationResult Debit(PlayerId player, CurrencyAmount amount, WalletReason reason) override;
    // Hydration/compatibility boundary only. Gameplay code should prefer Credit/Debit.
    WalletMutationResult SetBalance(PlayerId player, CurrencyAmount amount, WalletReason reason);

private:
    const player::legacy::LegacyOnlinePlayerRegistry& registry_;
};

LegacyWalletAdapter& ServerLegacyWalletAdapter();
}
}
}
