#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
checks = [
    (ROOT / "Shared/gameplay/companions/CompanionProtocol.h", "OPCODE_COMPANION_VISUAL_STATE", "visual opcode"),
    (ROOT / "Shared/gameplay/companions/CompanionProtocol.h", "motionSequence", "explicit motion sequence"),
    (ROOT / "Shared/gameplay/companions/CompanionProtocol.h", "equipmentRevision", "explicit equipment revision"),
    (ROOT / "SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp", "SendVisualStateLocked", "authoritative visual sender"),
    (ROOT / "SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp", "100U", "10 Hz visual throttle"),
    (ROOT / "SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp", "kDeadVisualHoldMs", "death visual retention"),
    (ROOT / "SrcServer/src/systems/gameplay/character/core/character.cpp", "PublishVisualState(this)", "post-tick visual publication"),
    (ROOT / "SrcGame/src/systems/network/v2/legacy/LegacyClientGameplayPacketRouter.h", "OPCODE_COMPANION_VISUAL_STATE", "client visual dispatch"),
    (ROOT / "Shared/gameplay/character/character_legacy_client.inl", "namespace client_visual", "client visual controller"),
    (ROOT / "Shared/gameplay/character/character_legacy_client.inl", "PlayBuffCnt = character->PlayBuffPosi_End", "legacy visual buffer bypass"),
    (ROOT / "Shared/gameplay/character/character_legacy_client.inl", "appliedMotionSequence", "idempotent motion application"),
    (ROOT / "Shared/gameplay/character/character_legacy_client.inl", "appliedEquipmentRevision", "idempotent equipment application"),
    (ROOT / "Shared/gameplay/character/character_legacy_client.inl", "client_visual::Update(this)", "ActionPattern 99 visual takeover"),
]
for path, needle, label in checks:
    text = path.read_text(errors="replace")
    if needle not in text:
        raise SystemExit(f"FAIL: {label}: missing {needle} in {path.relative_to(ROOT)}")

protocol = (ROOT / "Shared/gameplay/companions/CompanionProtocol.h").read_text(errors="replace")
visual_start = protocol.index("struct CompanionVisualEntry")
visual_end = protocol.index("struct CompanionVisualStatePacket")
visual_entry = protocol[visual_start:visual_end]
for forbidden in ("frame;", "action;", "wStickItems"):
    if forbidden in visual_entry:
        raise SystemExit(f"FAIL: model-local legacy field leaked into visual protocol: {forbidden}")

print("Companion Visual Runtime V2 architecture: PASS")
