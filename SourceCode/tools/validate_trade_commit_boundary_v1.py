#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
CONTRACT = ROOT / "Shared/gameplay/commerce/v2/TradeService.h"
ADAPTER = ROOT / "SrcServer/src/systems/commerce/v2/legacy/LegacyTradeAdapter.cpp"
SERVER = ROOT / "Shared/server/legacy_runtime/OnSever_legacy_server.inl"
ORACLE = ROOT / "Shared/legacy_oracle/server/core/server/legacy_runtime/OnSever.cpp.inl"


def fail(message: str) -> None:
    print(f"trade commit boundary v1: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def function(text: str, name: str, next_name: str) -> str:
    start = text.find(name)
    if start < 0:
        fail(f"missing function marker: {name}")
    end = text.find(next_name, start)
    if end < 0:
        fail(f"cannot delimit function after: {name}")
    return text[start:end]


def main() -> None:
    for path in (CONTRACT, ADAPTER, SERVER, ORACLE):
        if not path.is_file():
            fail(f"missing {path.relative_to(ROOT)}")

    contract = CONTRACT.read_text(encoding="utf-8", errors="replace")
    adapter = ADAPTER.read_text(encoding="utf-8", errors="replace")
    server = function(
        SERVER.read_text(encoding="utf-8", errors="replace"),
        "int\trsTradeInvenItem(",
        "int SendChatMessage()",
    )
    oracle = function(
        ORACLE.read_text(encoding="utf-8", errors="replace"),
        "int\trsTradeInvenItem(",
        "int SendChatMessage()",
    )

    for token in (
        "enum class TradeGoldCommitCode",
        "struct TradeGoldCommitRequest",
        "struct TradeGoldCommitResult",
        "InsufficientFunds",
        "ReceiverCapacityExceeded",
        "RollbackFailed",
        "virtual TradeGoldCommitResult CommitGold",
    ):
        if token not in contract:
            fail(f"trade commit contract missing: {token}")

    for forbidden in ("rsPLAYINFO", "ServerMoney", "WHERE_TRADE", "SubServerMoney", "AddServerMoney"):
        if forbidden in contract:
            fail(f"GREEN trade commit contract leaks legacy symbol: {forbidden}")

    for token in (
        "walletQuery_.TryGetWallet(request.sender, senderWallet)",
        "walletQuery_.TryGetWallet(request.receiver, receiverWallet)",
        "senderWallet.gold < request.amount",
        "receiverWallet.gold > kLegacyWalletMax - request.amount",
        "walletCommands_.Debit(request.sender, request.amount, WalletReason::Trade)",
        "walletCommands_.Credit(request.receiver, request.amount, WalletReason::Trade)",
        "walletCommands_.Credit(request.sender, request.amount, WalletReason::Trade)",
        "TradeGoldCommitCode::RollbackFailed",
        "TradeGoldCommitCode::Committed",
    ):
        if token not in adapter:
            fail(f"trade gold adapter safety token missing: {token}")

    for token in (
        "money = lpTransTradeItemKey->Money - 193;",
        "if(money < 0)",
        "if(money > 0)",
        "ServerLegacyTradeAdapter().CommitGold(",
        "TradeGoldCommitRequest(",
        "if(!goldCommit.Committed())",
        "RecordHackLogFile(lpPlayInfo, &smTransCommand);",
    ):
        if token not in server:
            fail(f"migrated trade commit token missing: {token}")

    for forbidden in (
        "lpPlayInfo->SubServerMoney(money, WHERE_TRADE)",
        "lpPlayInfo2->AddServerMoney(money, WHERE_TRADE)",
    ):
        if forbidden in server:
            fail(f"direct trade gold mutation remains in rsTradeInvenItem: {forbidden}")

    # Preserve the historical encoding and prove what was replaced.
    for token in (
        "money = lpTransTradeItemKey->Money - 193;",
        "if(lpPlayInfo->ServerMoney < money)",
        "lpPlayInfo->SubServerMoney(money, WHERE_TRADE);",
        "lpPlayInfo2->AddServerMoney(money, WHERE_TRADE);",
    ):
        if token not in oracle:
            fail(f"legacy oracle trade-gold behavior missing: {token}")

    print("trade commit boundary v1: PASS (Money-193 parity + wallet preflight/debit/credit + rollback guard)")


if __name__ == "__main__":
    main()
