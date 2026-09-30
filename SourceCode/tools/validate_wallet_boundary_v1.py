#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
CONTRACT = ROOT / "Shared/gameplay/commerce/v2/Wallet.h"
ADAPTER = ROOT / "SrcServer/src/systems/commerce/v2/legacy/LegacyWalletAdapter.cpp"
PLAYER = ROOT / "SrcServer/src/systems/gameplay/player/v2/legacy/LegacyPlayerAdapter.cpp"
LEGACY = ROOT / "SrcServer/src/core/server/legacy_runtime/PartySever.cpp"


def fail(message: str) -> None:
    print(f"wallet boundary v1: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    for path in (CONTRACT, ADAPTER, PLAYER, LEGACY):
        if not path.is_file():
            fail(f"missing {path.relative_to(ROOT)}")

    contract = CONTRACT.read_text(encoding="utf-8", errors="replace")
    adapter = ADAPTER.read_text(encoding="utf-8", errors="replace")
    player = PLAYER.read_text(encoding="utf-8", errors="replace")
    legacy = LEGACY.read_text(encoding="utf-8", errors="replace")

    for token in (
        "using CurrencyAmount = std::int64_t;",
        "struct WalletSnapshot",
        "enum class WalletReason",
        "enum class WalletMutationCode",
        "class IWalletQuery",
        "class IWalletCommands",
        "TryGetWallet(PlayerId player, WalletSnapshot& out)",
        "Credit(PlayerId player, CurrencyAmount amount, WalletReason reason)",
        "Debit(PlayerId player, CurrencyAmount amount, WalletReason reason)",
        "InsufficientFunds",
        "CapacityExceeded",
    ):
        if token not in contract:
            fail(f"GREEN wallet contract missing: {token}")
    for forbidden in ("rsPLAYINFO", "ServerMoney", "WHERE_", "onserver.h", "character.h"):
        if forbidden in contract:
            fail(f"GREEN wallet contract leaks legacy symbol: {forbidden}")

    for token in (
        "legacyPlayer->ServerMoney",
        "legacyPlayer->AddServerMoney(static_cast<int>(amount), LegacyWhere(reason))",
        "legacyPlayer->SubServerMoney(static_cast<int>(amount), LegacyWhere(reason))",
        "amount > kLegacyWalletMax - before",
        "if(before < amount)",
        "WalletMutationCode::InsufficientFunds",
        "WalletMutationCode::CapacityExceeded",
        "case WalletReason::Trade: return WHERE_TRADE;",
        "case WalletReason::NpcBuyItem: return WHERE_BUY_ITEM;",
        "case WalletReason::NpcSellItem: return WHERE_SELL_ITEM;",
        "case WalletReason::PersonalShop: return WHERE_PERSONAL_SHOP;",
        "case WalletReason::Crafting: return WHERE_CRAFTITEM;",
        "case WalletReason::Aging: return WHERE_AGINGITEM;",
    ):
        if token not in adapter:
            fail(f"legacy wallet parity/safety token missing: {token}")

    if "snapshot.gold = static_cast<std::int64_t>(player->ServerMoney);" not in player:
        fail("modern PlayerSnapshot does not expose authoritative live ServerMoney")
    if "snapshot.gold = static_cast<std::int64_t>(player->smCharInfo.Money);" in player:
        fail("modern PlayerSnapshot still reads stale record money")

    for token in (
        "int\trsPLAYINFO::AddServerMoney(int Money, int WhereParam)",
        "int\trsPLAYINFO::SubServerMoney(int Money, int WhereParam)",
        "ServerMoney += Money;",
        "ServerMoney -= Money;",
        "RecordHackLogFile(this, &smTransCommandEx);",
    ):
        if token not in legacy:
            fail(f"legacy authoritative wallet behavior missing: {token}")

    print("wallet boundary v1: PASS (live ServerMoney query + guarded credit/debit + typed audit reasons)")


if __name__ == "__main__":
    main()
