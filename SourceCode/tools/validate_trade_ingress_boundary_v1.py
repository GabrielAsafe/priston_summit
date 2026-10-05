#!/usr/bin/env python3
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
CONTRACT = ROOT / "Shared/gameplay/commerce/v2/TradeService.h"
ADAPTER = ROOT / "SrcServer/src/systems/commerce/v2/legacy/LegacyTradeAdapter.cpp"
INGRESS = ROOT / "Shared/server/legacy_runtime/OnSever_legacy_server.inl"
ORACLE = ROOT / "Shared/legacy_oracle/server/core/server/legacy_runtime/OnSever.cpp.inl"


def fail(message: str) -> None:
    print(f"trade ingress boundary v1: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def trade_case(text: str) -> str:
    marker = "case OpCode::OPCODE_TRADE_REQUST:"
    start = text.find(marker)
    if start < 0:
        fail("missing OPCODE_TRADE_REQUST handler")
    end = text.find("case OpCode::OPCODE_PARTY_JOIN:", start)
    if end < 0:
        fail("cannot delimit OPCODE_TRADE_REQUST handler")
    return text[start:end]


def main() -> None:
    for path in (CONTRACT, ADAPTER, INGRESS, ORACLE):
        if not path.is_file():
            fail(f"missing {path.relative_to(ROOT)}")

    contract = CONTRACT.read_text(encoding="utf-8", errors="replace")
    adapter = ADAPTER.read_text(encoding="utf-8", errors="replace")
    ingress = trade_case(INGRESS.read_text(encoding="utf-8", errors="replace"))
    oracle = trade_case(ORACLE.read_text(encoding="utf-8", errors="replace"))

    for token in (
        "enum class TradeAction",
        "Request = 0",
        "Accept = 1",
        "Open = 2",
        "Cancel = 3",
        "Unknown = 255",
        "TradeActionFromLegacyFlag",
        "enum class TradeIngressCode",
        "struct TradeIngressRequest",
        "struct TradeIngressResult",
        "class ITradeService",
        "EvaluateIngress(const TradeIngressRequest& request)",
        "bool ShouldRelay() const",
    ):
        if token not in contract:
            fail(f"GREEN trade contract missing: {token}")

    for forbidden in (
        "rsPLAYINFO",
        "smTRANS_COMMAND",
        "dwFuncChkErrorFlag",
        "szServerID",
        "DisableThrowItem",
        "RecvGetItemCoin",
        "smwsock.h",
        "onserver.h",
    ):
        if forbidden in contract:
            fail(f"GREEN trade contract leaks legacy symbol: {forbidden}")

    parity_tokens = (
        "sender->dwFuncChkErrorFlag || receiver->dwFuncChkErrorFlag",
        "sender->szServerID[0]",
        "rsServerConfig.DisableThrowItem",
        "cCoinShop.RecvGetItemCoin(sender);",
        "cCoinShop.RecvGetItemCoin(receiver);",
        "result.code = TradeIngressCode::Relay;",
    )
    for token in parity_tokens:
        if token not in adapter:
            fail(f"legacy trade ingress parity token missing: {token}")

    for token in (
        "ServerLegacyTradeAdapter().EvaluateIngress(tradeRequest)",
        "TradeActionFromLegacyFlag(lpTransCommand->LParam)",
        "lpTransCommand->LParam",
        "SendOnServerLegacySocket(lpPlayInfo2->lpsmSock, (char *)lpTransCommand, lpTransCommand->size);",
    ):
        if token not in ingress:
            fail(f"migrated ingress token missing: {token}")

    for forbidden in (
        "dwFuncChkErrorFlag",
        "szServerID[0]",
        "DisableThrowItem",
        "RecvGetItemCoin",
        "srFindUserFromSerial(lpTransCommand->WParam)",
    ):
        if forbidden in ingress:
            fail(f"legacy trade decision still owned by ingress handler: {forbidden}")

    # The oracle proves the ordering/semantics being preserved by the adapter:
    # receiver exists -> function blockers -> server/trade blocker -> coin flush -> relay.
    oracle_order = (
        "srFindUserFromSerial(lpTransCommand->WParam)",
        "dwFuncChkErrorFlag",
        "szServerID[0] || rsServerConfig.DisableThrowItem",
        "cCoinShop.RecvGetItemCoin(lpPlayInfo);",
        "cCoinShop.RecvGetItemCoin(lpPlayInfo2);",
        "lpsmSock->Send((char *)lpTransCommand, lpTransCommand->size, TRUE);",
    )
    positions = []
    for token in oracle_order:
        pos = oracle.find(token)
        if pos < 0:
            fail(f"legacy oracle no longer contains expected trade behavior: {token}")
        positions.append(pos)
    if positions != sorted(positions):
        fail("legacy oracle trade ordering changed unexpectedly")

    # Unknown flags intentionally remain relay-eligible: the old server did not
    # reject on LParam and preserving that behavior avoids a protocol change.
    if re.search(r"action\s*==|switch\s*\(\s*request\.action", adapter):
        fail("adapter unexpectedly gates ingress by parsed TradeAction")

    print("trade ingress boundary v1: PASS (typed action + legacy blockers/coin flush isolated + original packet relay preserved)")


if __name__ == "__main__":
    main()
