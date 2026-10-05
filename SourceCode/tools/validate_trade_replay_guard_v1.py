#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
CONTRACT = ROOT / "Shared/gameplay/commerce/v2/TradeService.h"
ADAPTER = ROOT / "SrcServer/src/systems/commerce/v2/legacy/LegacyTradeAdapter.cpp"
SERVER = ROOT / "Shared/server/legacy_runtime/OnSever_legacy_server.inl"
ORACLE = ROOT / "Shared/legacy_oracle/server/core/server/legacy_runtime/OnSever.cpp.inl"


def fail(message: str) -> None:
    print(f"trade replay guard v1: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def function(text: str, start_marker: str, end_marker: str) -> str:
    start = text.find(start_marker)
    if start < 0:
        fail(f"missing function marker: {start_marker}")
    end = text.find(end_marker, start)
    if end < 0:
        fail(f"cannot delimit function after: {start_marker}")
    return text[start:end]


def main() -> None:
    for path in (CONTRACT, ADAPTER, SERVER, ORACLE):
        if not path.is_file():
            fail(f"missing {path.relative_to(ROOT)}")

    contract = CONTRACT.read_text(encoding="utf-8", errors="replace")
    adapter = ADAPTER.read_text(encoding="utf-8", errors="replace")
    server = function(SERVER.read_text(encoding="utf-8", errors="replace"), "int\trsTradeInvenItem(", "int SendChatMessage()")
    oracle = function(ORACLE.read_text(encoding="utf-8", errors="replace"), "int\trsTradeInvenItem(", "int SendChatMessage()")

    for token in (
        "struct TradeId",
        "kTradeOfferItemSlots = 20u",
        "struct TradeCommitFingerprint",
        "BuildTradeId(const TradeCommitFingerprint& fingerprint)",
        "enum class TradeCommitGuardCode",
        "Duplicate",
        "struct TradeCommitGuardRequest",
        "CheckCommitReplay(const TradeCommitGuardRequest& request)",
        "MarkCommitted(const TradeCommitGuardRequest& request)",
    ):
        if token not in contract:
            fail(f"trade replay contract missing: {token}")

    for forbidden in ("rsPLAYINFO", "dwLastTradeTime", "dwLastTradeChksum", "MAX_TRADE_ITEM"):
        if forbidden in contract:
            fail(f"GREEN replay contract leaks legacy symbol: {forbidden}")

    session = (ROOT / "Shared/gameplay/commerce/v2/TradeSessionService.h").read_text(encoding="utf-8", errors="replace")
    for token in (
        "duplicateWindowMs = 8000u",
        "TradeCommitGuardCode::Duplicate",
        "BuildTradeId(fingerprint)",
    ):
        if token not in session:
            fail(f"replay service token missing: {token}")
    for token in ("replayGuard_.Check(", "replayGuard_.MarkCommitted(", "TradeReplayGuardService replayGuard(8000u)"):
        if token not in adapter:
            fail(f"legacy replay adapter delegation missing: {token}")

    for token in (
        "TradeCommitGuardRequest tradeGuardRequest",
        "fingerprint.encodedMoney",
        "fingerprint.items[cnt].itemCode",
        "ServerLegacyTradeAdapter().CheckCommitReplay(tradeGuardRequest)",
        "if(tradeGuard.Duplicate())",
        "ServerLegacyTradeAdapter().MarkCommitted(tradeGuardRequest)",
        "*TRADE CHECKING ERROR*",
    ):
        if token not in server:
            fail(f"migrated trade replay token missing: {token}")

    for forbidden in (
        "lpPlayInfo->dwLastTradeTime",
        "lpPlayInfo->dwLastTradeChksum",
        "dwTradeCode = (lpTransTradeItemKey->Money",
    ):
        if forbidden in server:
            fail(f"trade replay authority remains direct in server function: {forbidden}")

    for token in (
        "dwTradeCode = (lpTransTradeItemKey->Money * lpPlayInfo->dwObjectSerial + lpPlayInfo2->dwObjectSerial)",
        "(dwTradeTime - lpPlayInfo->dwLastTradeTime) < 8000",
        "lpPlayInfo->dwLastTradeChksum == dwTradeCode",
        "*TRADE CHECKING ERROR*",
        "lpPlayInfo->dwLastTradeTime = dwTradeTime",
        "lpPlayInfo->dwLastTradeChksum = dwTradeCode",
    ):
        if token not in oracle:
            fail(f"legacy oracle replay behavior missing: {token}")

    print("trade replay guard v1: PASS (TradeId checksum + 8s duplicate window + anti-dupe cleanup preserved)")


if __name__ == "__main__":
    main()
