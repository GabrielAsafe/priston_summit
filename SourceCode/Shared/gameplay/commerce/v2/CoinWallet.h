#pragma once

#include <cstdint>

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace commerce
{
using CoinAmount = std::int64_t;

enum class CoinReason
{
    Trade = 0,
    PersonalShop,
    QuestReward,
    Pickup,
    Administration,
    PremiumPurchase,
    PremiumPurchaseRollback
};

enum class CoinMutationCode
{
    Ok = 0,
    InvalidAmount,
    PlayerNotFound,
    InsufficientFunds,
    CapacityExceeded,
    LegacyRejected
};

struct CoinWalletSnapshot
{
    PlayerId player;
    CoinAmount coins;

    CoinWalletSnapshot() : player(), coins(0) {}
    CoinWalletSnapshot(PlayerId playerValue, CoinAmount coinsValue)
        : player(playerValue), coins(coinsValue) {}
};

struct CoinMutationResult
{
    CoinMutationCode code;
    PlayerId player;
    CoinAmount requested;
    CoinAmount before;
    CoinAmount after;

    CoinMutationResult()
        : code(CoinMutationCode::LegacyRejected), player(), requested(0), before(0), after(0) {}

    bool Ok() const { return code == CoinMutationCode::Ok; }
};

class ICoinWalletQuery
{
public:
    virtual ~ICoinWalletQuery() {}
    virtual bool TryGetCoinWallet(PlayerId player, CoinWalletSnapshot& out) const = 0;
};

class ICoinWalletCommands
{
public:
    virtual ~ICoinWalletCommands() {}
    virtual CoinMutationResult CreditCoins(PlayerId player, CoinAmount amount, CoinReason reason) = 0;
    virtual CoinMutationResult DebitCoins(PlayerId player, CoinAmount amount, CoinReason reason) = 0;
};
}
}
