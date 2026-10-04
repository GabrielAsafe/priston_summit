#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
CONTRACT = ROOT / "Shared/gameplay/social/v2/ChatDelivery.h"
ADAPTER = ROOT / "SrcServer/src/systems/community/chat/v2/legacy/LegacyChatDeliveryAdapter.cpp"
SERVER = ROOT / "Shared/server/legacy_runtime/OnSever_legacy_server.inl"
ORACLE = ROOT / "Shared/legacy_oracle/server/core/server/legacy_runtime/OnSever.cpp.inl"


def fail(message: str) -> None:
    print(f"chat delivery boundary v1: FAIL: {message}", file=sys.stderr)
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
    migrated = section(server, "int rsSendMessageAll(", "#define srHAMMER_MONSTER_MAX")
    oracle = section(
        ORACLE.read_text(encoding="utf-8", errors="replace"),
        "int rsSendMessageAll(",
        "#define srHAMMER_MONSTER_MAX",
    )

    for token in (
        "enum class ChatDeliveryKind",
        "Alert",
        "Whisper",
        "enum class ChatDeliveryCode",
        "struct ChatMessage",
        "struct ChatDeliveryResult",
        "class IChatDeliveryService",
        "SendToPlayer(PlayerId player, const ChatMessage& message)",
        "Broadcast(const ChatMessage& message)",
    ):
        if token not in contract:
            fail(f"GREEN chat contract missing: {token}")

    for forbidden in (
        "TRANS_CHATMESSAGE",
        "rsPLAYINFO",
        "lpsmSock",
        "OpCode::",
        "smwsock.h",
    ):
        if forbidden in contract:
            fail(f"GREEN chat contract leaks legacy symbol: {forbidden}")

    for token in (
        "TRANS_CHATMESSAGE packet = {};",
        "OpCode::SEND_ALERT",
        "OpCode::OPCODE_WHISPERMESSAGE",
        "packet.size = 32 + lstrlen(packet.szMessage)",
        "packet.dwIP = static_cast<DWORD>(message.color)",
        "packet.dwObjectSerial = 0",
        "registry_.Resolve(player)",
        "for(int index = 0; index < CONNECTMAX; ++index)",
        "rsPLAYINFO* legacyPlayer = &rsPlayInfo[index]",
        "packets_.Send(",
        "network::PacketView(&packet",
        "std::snprintf(packet.szMessage",
    ):
        if token not in adapter:
            fail(f"legacy chat adapter token missing: {token}")

    for token in (
        "ServerLegacyChatDeliveryAdapter().Broadcast(",
        "ChatDeliveryKind::Whisper",
        "ChatDeliveryKind::Alert",
        "ServerLegacyChatDeliveryAdapter().SendToLegacyPlayer(",
        "_vsnprintf_s(szMessage, _countof(szMessage), _TRUNCATE, msg, vl)",
    ):
        if token not in migrated:
            fail(f"migrated chat helper token missing: {token}")

    if migrated.count("ServerLegacyChatDeliveryAdapter().Broadcast(") != 2:
        fail("expected exactly two broadcast helper delegations")
    if migrated.count("ServerLegacyChatDeliveryAdapter().SendToLegacyPlayer(") != 2:
        fail("expected exactly two single-player helper delegations")

    for forbidden in (
        "TRANS_CHATMESSAGE",
        "lpsmSock->Send",
        "rsPlayInfo[cnt]",
        "OpCode::SEND_ALERT",
        "OpCode::OPCODE_WHISPERMESSAGE",
    ):
        if forbidden in migrated:
            fail(f"central chat helpers retain direct delivery authority: {forbidden}")

    for token in (
        "TRANS_CHATMESSAGE",
        "OpCode::SEND_ALERT",
        "OpCode::OPCODE_WHISPERMESSAGE",
        "lpsmSock->Send",
    ):
        if token not in oracle:
            fail(f"legacy chat oracle token missing: {token}")

    print(
        "chat delivery boundary v1: PASS "
        "(central alert/whisper single+broadcast delivery delegated; formatting preserved)"
    )


if __name__ == "__main__":
    main()
