#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
checks = [
    ("protocol action opcode", ROOT / "Shared/gameplay/companions/CompanionProtocol.h", "OPCODE_COMPANION_UI_ACTION"),
    ("protocol state opcode", ROOT / "Shared/gameplay/companions/CompanionProtocol.h", "OPCODE_COMPANION_UI_STATE"),
    ("server action handler", ROOT / "SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp", "void HandleUIAction"),
    ("server state sender", ROOT / "SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp", "void SendUIState"),
    ("session loadout", ROOT / "SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp", "LoadoutOverride"),
    ("DAT equipment scan", ROOT / "SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp", "ReadEquipmentSnapshot"),
    ("client companions page", ROOT / "SrcGame/src/systems/ui/v2/pages/CompanionsPage.cpp", "COMPANIONS DA CONTA"),
    ("structured gateway", ROOT / "SrcGame/src/systems/ui/v2/legacy/LegacyCompanionGateway.cpp", "CompanionUIActionPacket"),
    ("client packet handler", ROOT / "SrcGame/src/systems/network/client/netplay.cpp", "OPCODE_COMPANION_UI_STATE"),
    ("server packet handler", ROOT / "Shared/server/legacy_runtime/OnSever_legacy_server.inl", "OPCODE_COMPANION_UI_ACTION"),
    ("quick body preservation", ROOT / "Shared/gameplay/character/character_legacy_client.inl", "previousBody"),
    ("quick head preservation", ROOT / "Shared/gameplay/character/character_legacy_client.inl", "companionModelNameCode2"),
]

failed = False
for label, path, marker in checks:
    if not path.exists() or marker not in path.read_text(errors="ignore"):
        print(f"[FAIL] {label}: missing {marker!r} in {path.relative_to(ROOT) if path.exists() else path}")
        failed = True

# UI gateway must not regress to chat-command emulation.
gateway = ROOT / "SrcGame/src/systems/ui/v2/legacy/LegacyCompanionGateway.cpp"
if gateway.exists() and "/companion" in gateway.read_text(errors="ignore"):
    print("[FAIL] structured gateway contains slash-command fallback")
    failed = True

if failed:
    sys.exit(1)
print("Companion UI/head/loadout V1.1 contracts: PASS")
