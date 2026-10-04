#!/usr/bin/env python3
from pathlib import Path

root = Path(__file__).resolve().parents[4]
files = (
    (root / "SrcGame/src/systems/network/client/netplay.cpp", "ClientLegacyBackendChannelSender"),
    (root / "SrcServer/src/systems/network/server_gameplay/netplay.cpp", "ServerLegacyBackendChannelSender"),
)

for path, accessor in files:
    text = path.read_text(encoding="utf-8", errors="replace")
    body = text.split("int\tSendCommandUser(int wParam, int lParam, int sParam, int eParam)", 1)[1].split("int\tSendCommand_AreaServer", 1)[0]
    if "OpCode::OPCODE_COMMAND_USER" not in body:
        raise SystemExit(f"netplay command-user guard: opcode changed in {path.name}")
    if f"{accessor}().Send(" not in body or "BackendChannel::User" not in body:
        raise SystemExit(f"netplay command-user guard: typed User route missing in {path.name}")
    if "smWsockUserServer->Send" in body:
        raise SystemExit(f"netplay command-user guard: direct User send remains in {path.name}")
    if ".Succeeded() ? TRUE : FALSE" not in body:
        raise SystemExit(f"netplay command-user guard: legacy boolean return mapping changed in {path.name}")

print("netplay command User route guard: PASS")
