#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
CONTRACT = ROOT / "Shared/gameplay/commerce/v2/ShopService.h"
ADAPTER = ROOT / "SrcServer/src/systems/commerce/v2/legacy/LegacyShopAdapter.cpp"
SERVER = ROOT / "Shared/server/legacy_runtime/OnSever_legacy_server.inl"
ORACLE = ROOT / "Shared/legacy_oracle/server/core/server/legacy_runtime/OnSever.cpp.inl"


def fail(message: str) -> None:
    print(f"npc shop sale boundary v1: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def section(text: str, start_token: str, end_token: str) -> str:
    start = text.find(start_token)
    if start < 0:
        fail(f"missing section start: {start_token}")
    end = text.find(end_token, start)
    if end < 0:
        fail(f"missing section end: {end_token}")
    return text[start:end]


def main() -> None:
    for path in (CONTRACT, ADAPTER, SERVER, ORACLE):
        if not path.is_file():
            fail(f"missing {path.relative_to(ROOT)}")

    contract = CONTRACT.read_text(encoding="utf-8", errors="replace")
    adapter = ADAPTER.read_text(encoding="utf-8", errors="replace")
    sale = section(
        SERVER.read_text(encoding="utf-8", errors="replace"),
        "int rsRecvSellShopItem(",
        "int\trsCompareItems(",
    )
    oracle = section(
        ORACLE.read_text(encoding="utf-8", errors="replace"),
        "int rsRecvSellShopItem(",
        "int\trsCompareItems(",
    )

    for token in (
        "enum class ShopSaleCode",
        "RolledBack",
        "struct ShopSaleRequest",
        "struct ShopSaleResult",
        "CreditSale(const ShopSaleRequest& request)",
        "RollbackSale(const ShopSaleRequest& request)",
    ):
        if token not in contract:
            fail(f"GREEN sale contract missing: {token}")

    for forbidden in ("rsPLAYINFO", "ServerMoney", "WHERE_SELL_ITEM", "AddServerMoney", "rsDeleteInvenItem"):
        if forbidden in contract:
            fail(f"GREEN shop sale contract leaks legacy symbol: {forbidden}")

    for token in (
        "walletQuery_.TryGetWallet(request.player, wallet)",
        "wallet.gold > kLegacyWalletMax - request.amount",
        "walletCommands_.Credit(request.player, request.amount, WalletReason::NpcSellItem)",
        "walletCommands_.Debit(request.player, request.amount, WalletReason::NpcSellItem)",
        "ShopSaleCode::RolledBack",
        "ShopSaleCode::RollbackRejected",
    ):
        if token not in adapter:
            fail(f"shop sale adapter token missing: {token}")

    for token in (
        "Price2 = (int)((Price*(Dur[0] / Dur[1]))",
        '"vendor.blacksmith_sell_value"',
        "ServerLegacyShopAdapter().SellItem(",
        "if(!sale.Sold())",
        "ShopItemSaleCode::ItemNotFound",
        "ShopItemSaleCode::RollbackFailed",
        "SaveThrowItem(lpPlayInfo",
        "ITEMLOG_SELLSHOP",
    ):
        if token not in sale:
            fail(f"migrated sale path token missing: {token}")

    if "lpPlayInfo->AddServerMoney(smTransCommand.EParam, WHERE_SELL_ITEM)" in sale:
        fail("direct shop sale wallet credit remains in legacy function")

    sell_pos = sale.find("ServerLegacyShopAdapter().SellItem(")
    failure_pos = sale.find("if(!sale.Sold())")
    save_pos = sale.find("SaveThrowItem(lpPlayInfo")
    if not (0 <= sell_pos < failure_pos < save_pos):
        fail("sale commit/failure/log ordering is not defensive")

    if "rsDeleteInvenItem(lpPlayInfo" in sale:
        fail("direct inventory delete remains in migrated sale handler")

    for token in (
        "Price2 = (int)((Price*(Dur[0] / Dur[1]))",
        "lpPlayInfo->AddServerMoney(smTransCommand.EParam, WHERE_SELL_ITEM);",
        "ITEMLOG_SELLSHOP",
    ):
        if token not in oracle:
            fail(f"legacy sale oracle token missing: {token}")

    print("npc shop sale boundary v1: PASS (legacy price/covenant formula + atomic inventory/wallet sale)")


if __name__ == "__main__":
    main()
