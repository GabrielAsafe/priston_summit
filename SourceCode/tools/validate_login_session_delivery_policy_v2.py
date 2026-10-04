#!/usr/bin/env python3
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[1]
policy_path = root / "Shared/authentication/v2/LoginSessionDeliveryPolicy.h"
record_path = root / "Shared/persistence/character_record/record_legacy_server.inl"
sender_path = root / "SrcServer/src/systems/network/v2/legacy/LegacyPacketSender.cpp"

policy = policy_path.read_text(encoding="utf-8", errors="replace") if policy_path.exists() else ""
record = record_path.read_text(encoding="utf-8", errors="replace")
sender = sender_path.read_text(encoding="utf-8", errors="replace")
record_fn = record.split("int rsRECORD_DBASE::SendRecordDataToClient", 1)[1].split("#define\tRECORD_ITEM_INFO_HEAD_SIZE", 1)[0]

checks = {
    "delivery policy exists": policy_path.exists(),
    "policy is socket agnostic": "smWINSOCK" not in policy and "rsPLAYINFO" not in policy,
    "stable player route explicit": "RegisteredPlayer" in policy and "HasStablePlayerIdentity()" in policy,
    "pre-registration route explicit": "TransientSession" in policy,
    "character record keeps caller socket": "SummitRecordSocketSend(lpsmSock" in record_fn,
    "character record avoids PlayerId lookup": "ServerLegacyPacketSender().Send(" not in record_fn,
    "amber sender retains explicit socket path": "SendToLegacySocket(smWINSOCK* socket" in sender,
}

failed = [name for name, ok in checks.items() if not ok]
if failed:
    for name in failed:
        print(f"FAIL: {name}", file=sys.stderr)
    sys.exit(1)
print("login session delivery policy v2 validator: PASS")
