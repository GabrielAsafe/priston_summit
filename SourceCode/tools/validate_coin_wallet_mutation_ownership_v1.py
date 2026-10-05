#!/usr/bin/env python3
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
COIN_KERNEL = ROOT / "SrcServer/src/systems/commerce/coin_shop/CoinShop.cpp"
TIME_KERNEL = ROOT / "SrcServer/src/systems/commerce/coin_shop/CoinShop_T.cpp"
COIN_ADAPTER = ROOT / "SrcServer/src/systems/commerce/v2/legacy/LegacyCoinWalletAdapter.cpp"
TIME_ADAPTER = ROOT / "SrcServer/src/systems/commerce/v2/legacy/LegacyTimeCreditWalletAdapter.h"
ONSERVER = ROOT / "Shared/server/legacy_runtime/OnSever_legacy_server.inl"
QUEST = ROOT / "SrcServer/src/systems/gameplay/quests/server/cQuestHandler.cpp"
NEWS = ROOT / "SrcServer/src/systems/community/news/News.cpp"

DIRECT_RE = re.compile(r"\bcCoinShop(?:_T)?\.(?:AddPlayerCoin|GetPlayerCoin)\s*\(")
ALLOWED = {COIN_KERNEL, TIME_KERNEL, COIN_ADAPTER, TIME_ADAPTER}


def fail(message: str) -> None:
    print(f"coin wallet mutation ownership v1: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def candidate_files():
    seen = set()
    for base in (ROOT / "SrcServer/src", ROOT / "Shared"):
        for path in base.rglob("*"):
            if not path.is_file() or path.suffix.lower() not in {".cpp", ".h", ".hpp", ".inl"}:
                continue
            rel = path.relative_to(ROOT).as_posix()
            if "/legacy_oracle/" in f"/{rel}/" or ".before" in path.name or "_legacy_client" in path.name:
                continue
            if path in ALLOWED or path in seen:
                continue
            seen.add(path)
            yield path


def main() -> None:
    for required in (COIN_KERNEL, TIME_KERNEL, COIN_ADAPTER, TIME_ADAPTER, ONSERVER, QUEST, NEWS):
        if not required.is_file():
            fail(f"missing {required.relative_to(ROOT)}")

    coin_adapter = COIN_ADAPTER.read_text(encoding="utf-8", errors="replace")
    time_adapter = TIME_ADAPTER.read_text(encoding="utf-8", errors="replace")
    for token in ("cCoinShop.GetPlayerCoin(legacyPlayer)", "cCoinShop.AddPlayerCoin(legacyPlayer"):
        if token not in coin_adapter:
            fail(f"SCoins adapter kernel token missing: {token}")
    for token in ("cCoinShop_T.GetPlayerCoin(legacyPlayer)", "cCoinShop_T.AddPlayerCoin(legacyPlayer"):
        if token not in time_adapter:
            fail(f"TCoin adapter kernel token missing: {token}")

    onserver = ONSERVER.read_bytes().decode("latin1")
    for token in (
        "ServerLegacyCoinWalletAdapter().CreditCoins(",
        "ServerLegacyCoinWalletAdapter().DebitCoins(",
        "ServerLegacyCoinWalletAdapter().TryGetCoinWallet(",
        "ServerLegacyTimeCreditWalletAdapter().CreditCoins(",
        "ServerLegacyTimeCreditWalletAdapter().TryGetCoinWallet(",
        "CoinReason::Pickup",
        "CoinReason::Administration",
    ):
        if token not in onserver:
            fail(f"OnServer wallet delegation token missing: {token}")

    quest = QUEST.read_text(encoding="utf-8", errors="replace")
    for token in (
        "ServerLegacyCoinWalletAdapter().CreditCoins(",
        "ServerLegacyTimeCreditWalletAdapter().CreditCoins(",
        "CoinReason::QuestReward",
    ):
        if token not in quest:
            fail(f"quest reward wallet delegation token missing: {token}")

    news = NEWS.read_text(encoding="utf-8", errors="replace")
    for token in (
        "ServerLegacyCoinWalletAdapter().TryGetCoinWallet(",
        "ServerLegacyTimeCreditWalletAdapter().TryGetCoinWallet(",
    ):
        if token not in news:
            fail(f"news wallet query token missing: {token}")

    unauthorized = []
    for path in candidate_files():
        text = path.read_text(encoding="utf-8", errors="replace")
        for lineno, line in enumerate(text.splitlines(), 1):
            if DIRECT_RE.search(line):
                unauthorized.append(f"{path.relative_to(ROOT)}:{lineno}: {line.strip()}")
    if unauthorized:
        fail("direct SCoins/TCoin wallet access remains outside adapters/kernel:\n" + "\n".join(unauthorized))

    print("coin wallet mutation ownership v1: PASS (SCoins/TCoin query+mutation authority centralized, unauthorized=0)")


if __name__ == "__main__":
    main()
