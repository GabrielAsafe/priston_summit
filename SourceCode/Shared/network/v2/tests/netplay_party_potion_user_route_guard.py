#!/usr/bin/env python3
from pathlib import Path

root = Path(__file__).resolve().parents[4]
files = (
    (root / "SrcGame/src/systems/network/client/netplay.cpp", "ClientLegacyBackendChannelSender"),
    (root / "SrcServer/src/systems/network/server_gameplay/netplay.cpp", "ServerLegacyBackendChannelSender"),
)

for path, accessor in files:
    text = path.read_text(encoding="utf-8", errors="replace")
    body = text.rsplit("int ThrowItemToServer(smTRANS_COMMAND_EX *lpTransCommand)", 1)[1].split("if(!TransThrowItem.Item.SpecialItemFlag[0])", 1)[0]
    if "OpCode::OPCODE_PARTY_POTION" not in body:
        raise SystemExit(f"netplay party potion route guard: opcode missing in {path.name}")
    if f"{accessor}().Send(" not in body or "BackendChannel::User" not in body:
        raise SystemExit(f"netplay party potion route guard: typed User route missing in {path.name}")
    if "smWsockUserServer->Send" in body:
        raise SystemExit(f"netplay party potion route guard: direct User send remains in {path.name}")
    if "SetPartyPosion(&smTransCommand);" not in body:
        raise SystemExit(f"netplay party potion route guard: legacy local party side effect changed in {path.name}")

print("netplay party potion User route guard: PASS")
