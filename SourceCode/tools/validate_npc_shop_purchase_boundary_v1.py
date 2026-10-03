#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
CONTRACT = ROOT / "Shared/gameplay/commerce/v2/ShopService.h"
ADAPTER = ROOT / "SrcServer/src/systems/commerce/v2/legacy/LegacyShopAdapter.cpp"
KERNEL = ROOT / "Shared/gameplay/commerce/v2/ShopTransactionService.h"
SERVER = ROOT / "Shared/server/legacy_runtime/OnSever_legacy_server.inl"
ORACLE = ROOT / "Shared/legacy_oracle/server/core/server/legacy_runtime/OnSever.cpp.inl"


def fail(message: str) -> None:
    print(f"npc shop purchase boundary v1: FAIL: {message}", file=sys.stderr)
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
    for path in (CONTRACT, ADAPTER, KERNEL, SERVER, ORACLE):
        if not path.is_file():
            fail(f"missing {path.relative_to(ROOT)}")

    contract = CONTRACT.read_text(encoding="utf-8", errors="replace")
    adapter = ADAPTER.read_text(encoding="utf-8", errors="replace")
    kernel = KERNEL.read_text(encoding="utf-8", errors="replace")
    buy = section(
        SERVER.read_text(encoding="utf-8", errors="replace"),
        "int rsRecvBuyShopItem(",
        "int rsRecvSellShopItem(",
    )
    oracle = section(
        ORACLE.read_text(encoding="utf-8", errors="replace"),
        "int rsRecvBuyShopItem(",
        "int rsRecvSellShopItem(",
    )

    for token in (
        "enum class ShopPurchaseKind",
        "enum class ShopPurchaseCode",
        "struct ShopPurchaseRequest",
        "struct ShopPurchaseResult",
        "class IShopService",
        "virtual ShopPurchaseResult Purchase",
        "TotalOverflow",
        "InsufficientFunds",
    ):
        if token not in contract:
            fail(f"GREEN shop contract missing: {token}")

    for forbidden in ("rsPLAYINFO", "ServerMoney", "WHERE_BUY", "SubServerMoney", "sITEMINFO"):
        if forbidden in contract:
            fail(f"GREEN shop contract leaks legacy symbol: {forbidden}")

    for token in (
        "std::numeric_limits<CurrencyAmount>::max() / request.unitPrice",
        "result.total = request.unitPrice * static_cast<CurrencyAmount>(request.quantity);",
        "walletQuery_.TryGetWallet(request.player, wallet)",
        "if(wallet.gold < result.total)",
        "WalletReason::NpcBuyPotion",
        "WalletReason::NpcBuyItem",
        "walletCommands_.Debit(request.player, result.total, reason)",
    ):
        if token not in kernel:
            fail(f"shop purchase kernel token missing: {token}")

    for token in (
        "ServerLegacyShopAdapter().Purchase(",
        "ServerLegacyShopAdapter().PurchaseItem(",
        "ServerLegacyShopAdapter().PurchasePotion(",
        "const bool isPotionPurchase",
        "const int purchaseQuantity = isPotionPurchase ? lpTransBuyShopItem->ItemCount : 1;",
        "if(!purchased)",
        "money = static_cast<int>(purchaseTotal);",
        "CurrencyLog.int64_ShopMoney",
    ):
        if token not in buy:
            fail(f"migrated buy path token missing: {token}")

    if "rsAddServerPotion(" in buy:
        fail("direct potion stack authority remains in shop buy handler")

    for forbidden in (
        "SubServerMoney(money, WHERE_BUY_POTION)",
        "SubServerMoney(lpTransBuyShopItem->sItem.sItemInfo.Price, WHERE_BUY_ITEM)",
        "lpTransBuyShopItem->sItem.sItemInfo.Price <= lpPlayInfo->ServerMoney",
    ):
        if forbidden in buy:
            fail(f"direct shop purchase wallet authority remains: {forbidden}")

    for token in (
        "lpTransBuyShopItem->sItem.sItemInfo.Price <= lpPlayInfo->ServerMoney",
        "money = lpTransBuyShopItem->sItem.sItemInfo.Price*lpTransBuyShopItem->ItemCount;",
        "lpPlayInfo->SubServerMoney(money, WHERE_BUY_POTION);",
        "lpPlayInfo->SubServerMoney(lpTransBuyShopItem->sItem.sItemInfo.Price, WHERE_BUY_ITEM);",
    ):
        if token not in oracle:
            fail(f"legacy shop oracle token missing: {token}")

    print("npc shop purchase boundary v1: PASS (potion wallet parity + atomic exact-item purchase boundary)")


if __name__ == "__main__":
    main()
