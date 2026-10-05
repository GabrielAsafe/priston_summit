#pragma once

#include <cstdint>

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace commerce
{
using CurrencyAmount = std::int64_t;

enum class WalletReason
{
    Trade = 0,
    GiftExpress,
    Aging,
    Crafting,
    ForceOrb,
    LearnSkill,
    WarpGate,
    NpcBuyItem,
    NpcBuyPotion,
    NpcSellItem,
    ThrowItem,
    PickupItem,
    StarPoint,
    ClanMoney,
    GiveMoney,
    PersonalShop,
    Warehouse,
    LoadPlayer,
    NpcShopCompatibility
};

enum class WalletMutationCode
{
    Ok = 0,
    InvalidAmount,
    PlayerNotFound,
    InsufficientFunds,
    CapacityExceeded,
    LegacyRejected
};

struct WalletSnapshot
{
    PlayerId player;
    CurrencyAmount gold;

    WalletSnapshot() : player(), gold(0) {}
    WalletSnapshot(PlayerId playerValue, CurrencyAmount goldValue)
        : player(playerValue), gold(goldValue) {}
};

struct WalletMutationResult
{
    WalletMutationCode code;
    PlayerId player;
    CurrencyAmount requested;
    CurrencyAmount before;
    CurrencyAmount after;

    WalletMutationResult()
        : code(WalletMutationCode::LegacyRejected), player(), requested(0), before(0), after(0) {}

    bool Ok() const { return code == WalletMutationCode::Ok; }
};

class IWalletQuery
{
public:
    virtual ~IWalletQuery() {}
    virtual bool TryGetWallet(PlayerId player, WalletSnapshot& out) const = 0;
};

class IWalletCommands
{
public:
    virtual ~IWalletCommands() {}
    virtual WalletMutationResult Credit(PlayerId player, CurrencyAmount amount, WalletReason reason) = 0;
    virtual WalletMutationResult Debit(PlayerId player, CurrencyAmount amount, WalletReason reason) = 0;
};
}
}
