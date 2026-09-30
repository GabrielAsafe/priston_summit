#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
CONTRACT = ROOT / "Shared/gameplay/commerce/v2/PersonalShopService.h"
ADAPTER = ROOT / "SrcServer/src/systems/commerce/v2/legacy/LegacyPersonalShopAdapter.cpp"
SERVER = ROOT / "Shared/server/legacy_runtime/OnSever_legacy_server.inl"
ORACLE = ROOT / "Shared/legacy_oracle/server/core/server/legacy_runtime/OnSever.cpp.inl"


def fail(message: str) -> None:
    print(f"personal shop ingress boundary v1: FAIL: {message}", file=sys.stderr)
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
    server = SERVER.read_text(encoding="utf-8", errors="replace")
    migrated = section(server, "case OpCode::OPCODE_OPEN_MYSHOP:", "case OpCode::OPCODE_MYSHOP_MESSAGE:")
    oracle = section(
        ORACLE.read_text(encoding="utf-8", errors="replace"),
        "case OpCode::OPCODE_OPEN_MYSHOP:",
        "case OpCode::OPCODE_MYSHOP_MESSAGE:",
    )
    trade_commit = section(server, "int\trsTrade_PersonalShop(", "//Vl")

    for token in (
        "enum class PersonalShopIngressAction",
        "OpenListing",
        "CallListing",
        "RelayItem",
        "TradeRequest",
        "enum class PersonalShopIngressCode",
        "struct PersonalShopIngressRequest",
        "struct PersonalShopIngressResult",
        "class IPersonalShopService",
        "HandleIngress(const PersonalShopIngressRequest& request)",
    ):
        if token not in contract:
            fail(f"GREEN personal-shop contract missing: {token}")

    for forbidden in (
        "rsPLAYINFO",
        "szServerID",
        "DisableThrowItem",
        "srFindUserFromSerial",
        "lpsmSock",
        "sMYSHOP_ITEM_SERVER",
    ):
        if forbidden in contract:
            fail(f"GREEN personal-shop contract leaks legacy symbol: {forbidden}")

    for token in (
        "return action != PersonalShopIngressAction::RelayItem",
        "sender->szServerID[0]",
        "rsServerConfig.DisableThrowItem",
        "strcpy_s(sender->szTradeMessage, request.listingMessage.c_str())",
        "sender->dwTradeMsgCode = static_cast<DWORD>(request.listingCode)",
        "registry_.Resolve(request.receiver)",
        "registry_.HandleOf(receiver)",
    ):
        if token not in adapter:
            fail(f"legacy personal-shop adapter token missing: {token}")

    for token in (
        "PersonalShopIngressAction::OpenListing",
        "PersonalShopIngressAction::CallListing",
        "PersonalShopIngressAction::RelayItem",
        "PersonalShopIngressAction::TradeRequest",
        "ServerLegacyPersonalShopAdapter().HandleIngress",
        "Resolve(ingress.receiver)",
        "rsTrade_PersonalShop(lpPlayInfo, lpPlayInfo2",
    ):
        if token not in migrated:
            fail(f"migrated personal-shop ingress token missing: {token}")

    if migrated.count("ServerLegacyPersonalShopAdapter().HandleIngress") != 4:
        fail("expected exactly four personal-shop ingress delegations")

    for forbidden in (
        "srFindUserFromSerial(",
        "lpPlayInfo->szServerID[0]",
        "rsServerConfig.DisableThrowItem",
        "strcpy_s(lpPlayInfo->szTradeMessage",
        "lpPlayInfo->dwTradeMsgCode =",
    ):
        if forbidden in migrated:
            fail(f"personal-shop ingress still owns legacy validation/mutation: {forbidden}")

    # In RECOVERY_018 the authoritative transaction remained direct. Later recovery
    # points may delegate it through the same personal-shop boundary; both states keep
    # the ingress guarantees validated above.
    delegated_commit = "ServerLegacyPersonalShopAdapter().CommitTrade(request)" in trade_commit
    if not delegated_commit:
        for token in (
            "lpPlayInfo2->SubServerMoney(lpMyShopItem->Price, WHERE_PERSONAL_SHOP)",
            "lpPlayInfo->AddServerMoney(lpMyShopItem->Price, WHERE_PERSONAL_SHOP)",
            "rsDeleteInvenItem(lpPlayInfo",
            "rsAddInvenItem(lpPlayInfo2",
        ):
            if token not in trade_commit:
                fail(f"personal-shop commit missing legacy/delegated authority: {token}")

    for token in (
        "if(lpPlayInfo->szServerID[0] || rsServerConfig.DisableThrowItem) break;",
        "strcpy_s(lpPlayInfo->szTradeMessage, lpTransChatMessage->szMessage)",
        "lpPlayInfo2 = srFindUserFromSerial(lpTransCommand->LParam)",
        "lpPlayInfo2 = srFindUserFromSerial(lpTransTradeItems->dwRecver)",
    ):
        if token not in oracle:
            fail(f"legacy personal-shop ingress oracle token missing: {token}")

    print(
        "personal shop ingress boundary v1: PASS "
        "(open/call/item/trade ingress isolated; item-relay blocker asymmetry preserved)"
    )


if __name__ == "__main__":
    main()
