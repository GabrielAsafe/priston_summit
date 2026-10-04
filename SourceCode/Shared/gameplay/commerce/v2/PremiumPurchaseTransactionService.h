#pragma once

#include <string>
#include <utility>

#include "gameplay/commerce/v2/CoinWallet.h"

namespace gameplay::commerce
{
enum class PremiumPurchaseCode
{
    Purchased = 0,
    InvalidPlayer,
    InvalidPrice,
    WalletNotFound,
    InsufficientFunds,
    DebitRejected,
    GrantRejected,
    RollbackFailed
};

struct PremiumPurchaseRequest
{
    PlayerId player;
    CoinAmount price = 0;
    std::string itemCode;
};

struct PremiumPurchaseResult
{
    PremiumPurchaseCode code = PremiumPurchaseCode::DebitRejected;
    CoinAmount before = 0;
    CoinAmount after = 0;
    bool Purchased() const noexcept { return code == PremiumPurchaseCode::Purchased; }
};

class IPremiumPurchaseGrant
{
public:
    virtual ~IPremiumPurchaseGrant() = default;
    virtual bool Grant(const PremiumPurchaseRequest& request) = 0;
};

class PremiumPurchaseTransactionService
{
public:
    PremiumPurchaseTransactionService(const ICoinWalletQuery& walletQuery,
                                      ICoinWalletCommands& walletCommands,
                                      IPremiumPurchaseGrant& grant)
        : walletQuery_(walletQuery), walletCommands_(walletCommands), grant_(grant) {}

    PremiumPurchaseResult Purchase(const PremiumPurchaseRequest& request)
    {
        PremiumPurchaseResult result;
        if (!request.player.Valid()) { result.code = PremiumPurchaseCode::InvalidPlayer; return result; }
        if (request.price <= 0) { result.code = PremiumPurchaseCode::InvalidPrice; return result; }

        CoinWalletSnapshot wallet;
        if (!walletQuery_.TryGetCoinWallet(request.player, wallet))
        { result.code = PremiumPurchaseCode::WalletNotFound; return result; }
        result.before = wallet.coins;
        result.after = wallet.coins;
        if (wallet.coins < request.price)
        { result.code = PremiumPurchaseCode::InsufficientFunds; return result; }

        const CoinMutationResult debit = walletCommands_.DebitCoins(
            request.player, request.price, CoinReason::PremiumPurchase);
        result.after = debit.after;
        if (!debit.Ok())
        { result.code = PremiumPurchaseCode::DebitRejected; return result; }

        if (grant_.Grant(request))
        { result.code = PremiumPurchaseCode::Purchased; return result; }

        const CoinMutationResult rollback = walletCommands_.CreditCoins(
            request.player, request.price, CoinReason::PremiumPurchaseRollback);
        result.after = rollback.after;
        result.code = rollback.Ok() ? PremiumPurchaseCode::GrantRejected
                                    : PremiumPurchaseCode::RollbackFailed;
        return result;
    }

private:
    const ICoinWalletQuery& walletQuery_;
    ICoinWalletCommands& walletCommands_;
    IPremiumPurchaseGrant& grant_;
};
}
