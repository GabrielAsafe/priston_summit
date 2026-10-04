#!/usr/bin/env python3
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[1]
onserver = (root / "Shared/server/legacy_runtime/OnSever_legacy_server.inl").read_text(encoding="utf-8", errors="replace")
record = (root / "Shared/persistence/character_record/record_legacy_server.inl").read_text(encoding="utf-8", errors="replace")
registry = root / "SrcServer/src/systems/authentication/v2/legacy/LegacyLoginSessionRegistry.h"
state = root / "Shared/authentication/v2/LoginSessionState.h"

checks = {
    "state contract": state.exists(),
    "legacy registry": registry.exists(),
    "connection opens session": "ServerLegacyLoginSessionRegistry().Open(&rsPlayInfo[cnt])" in onserver,
    "login begins handshake": "ServerLegacyLoginSessionRegistry().BeginHandshake(playInfo)" in onserver,
    "auth publishes character list": "MarkAuthenticatedAndCharacterListReady(LogAccount->lpPlayInfo)" in onserver,
    "record request advances session": "BeginCharacterRecordLoad(lpPlayInfo)" in onserver,
    "player registration explicit": "ServerLegacyLoginSessionRegistry().RegisterPlayer(lpPlayInfo)" in onserver,
    "disconnect closes session": "ServerLegacyLoginSessionRegistry().Close(tpInfo)" in onserver,
    "record delivery remains explicit socket": "SummitRecordSocketSend(lpsmSock" in record,
    "record delivery does not resolve player id": "ServerLegacyPacketSender().Send(" not in record.split("int rsRECORD_DBASE::SendRecordDataToClient", 1)[1].split("#define\tRECORD_ITEM_INFO_HEAD_SIZE", 1)[0],
}

failed = [name for name, ok in checks.items() if not ok]
if failed:
    for name in failed:
        print(f"FAIL: {name}", file=sys.stderr)
    sys.exit(1)
print("login session lifecycle v2 validator: PASS")
