#pragma once

#include <cstdint>
#include <string>

#include "gameplay/commerce/v2/CoinWallet.h"

namespace gameplay::commerce
{
struct RestorationRecordSnapshot
{
    std::string account;
    std::string character;
    std::string itemName;
    int itemAging = 0;
    std::uint32_t head = 0u;
    std::uint32_t checksum = 0u;
};

struct RestorationTransactionRequest
{
    PlayerId player;
    CoinAmount price = 0;
    RestorationRecordSnapshot record;
};

enum class RestorationTransactionCode
{
    Restored = 0,
    InvalidPlayer,
    InvalidPrice,
    WalletNotFound,
    InsufficientFunds,
    DebitRejected,
    RecordConsumeRejected,
    GrantRejected,
    RollbackFailed
};

struct RestorationTransactionResult
{
    RestorationTransactionCode code = RestorationTransactionCode::RecordConsumeRejected;
    CoinAmount before = 0;
    CoinAmount after = 0;
    bool Restored() const noexcept { return code == RestorationTransactionCode::Restored; }
};

class IRestorationClaimStore
{
public:
    virtual ~IRestorationClaimStore() = default;
    virtual bool Consume(const RestorationRecordSnapshot& record) = 0;
    virtual bool Restore(const RestorationRecordSnapshot& record) = 0;
};

class IRestorationGrant
{
public:
    virtual ~IRestorationGrant() = default;
    virtual bool Grant(const RestorationRecordSnapshot& record) = 0;
};

class RestorationTransactionService final
{
public:
    RestorationTransactionService(const ICoinWalletQuery& walletQuery,
                                  ICoinWalletCommands& walletCommands,
                                  IRestorationClaimStore& claimStore,
                                  IRestorationGrant& grant)
        : walletQuery_(walletQuery), walletCommands_(walletCommands),
          claimStore_(claimStore), grant_(grant) {}

    RestorationTransactionResult Restore(const RestorationTransactionRequest& request)
    {
        RestorationTransactionResult result;
        if(!request.player.Valid())
        {
            result.code = RestorationTransactionCode::InvalidPlayer;
            return result;
        }
        if(request.price < 0)
        {
            result.code = RestorationTransactionCode::InvalidPrice;
            return result;
        }

        CoinWalletSnapshot wallet;
        if(!walletQuery_.TryGetCoinWallet(request.player, wallet))
        {
            result.code = RestorationTransactionCode::WalletNotFound;
            return result;
        }
        result.before = wallet.coins;
        result.after = wallet.coins;
        if(wallet.coins < request.price)
        {
            result.code = RestorationTransactionCode::InsufficientFunds;
            return result;
        }

        if(request.price > 0)
        {
            const CoinMutationResult debit = walletCommands_.DebitCoins(
                request.player, request.price, CoinReason::PremiumPurchase);
            result.after = debit.after;
            if(!debit.Ok())
            {
                result.code = RestorationTransactionCode::DebitRejected;
                return result;
            }
        }

        if(!claimStore_.Consume(request.record))
        {
            if(!Refund(request, result))
                result.code = RestorationTransactionCode::RollbackFailed;
            else
                result.code = RestorationTransactionCode::RecordConsumeRejected;
            return result;
        }

        if(grant_.Grant(request.record))
        {
            result.code = RestorationTransactionCode::Restored;
            return result;
        }

        const bool recordRestored = claimStore_.Restore(request.record);
        const bool walletRestored = Refund(request, result);
        result.code = (recordRestored && walletRestored)
            ? RestorationTransactionCode::GrantRejected
            : RestorationTransactionCode::RollbackFailed;
        return result;
    }

private:
    bool Refund(const RestorationTransactionRequest& request,
                RestorationTransactionResult& result)
    {
        if(request.price == 0)
            return true;
        const CoinMutationResult rollback = walletCommands_.CreditCoins(
            request.player, request.price, CoinReason::PremiumPurchaseRollback);
        result.after = rollback.after;
        return rollback.Ok();
    }

    const ICoinWalletQuery& walletQuery_;
    ICoinWalletCommands& walletCommands_;
    IRestorationClaimStore& claimStore_;
    IRestorationGrant& grant_;
};
}
