#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
checks = [
    ("Companion header", ROOT / "SrcServer/src/systems/gameplay/companions/CompanionSystem.h", "AwardExperience"),
    ("Companion implementation", ROOT / "SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp", "OpenCharacterCompanionFromRecord"),
    ("Account character loader", ROOT / "Shared/persistence/character_record/record_legacy_server.inl", "rsLoadAccountCharacterRecord"),
    ("Offline EXP persistence", ROOT / "Shared/persistence/character_record/record_legacy_server.inl", "rsAddOfflineCharacterExp"),
    ("Offline record integrity", ROOT / "Shared/persistence/character_record/record_legacy_server.inl", "ReformCharForm(&recordData.smCharInfo)"),
    ("Party EXP integration", ROOT / "SrcServer/src/core/server/legacy_runtime/PartySever.cpp", "GetPartyActiveCount"),
    ("Party companion award", ROOT / "SrcServer/src/core/server/legacy_runtime/PartySever.cpp", "companions::AwardExperience"),
    ("Solo companion EXP", ROOT / "Shared/server/legacy_runtime/OnSever_legacy_server.inl", "companionPartyRate"),
    ("Crystal bridge", ROOT / "Shared/server/legacy_runtime/OnSever_legacy_server.inl", "HandleCrystalUse"),
    ("Command bridge", ROOT / "Shared/server/legacy_runtime/OnSever_legacy_server.inl", 'rsCompString("/companion"'),
    ("Disconnect cleanup", ROOT / "Shared/server/legacy_runtime/OnSever_legacy_server.inl", "OnOwnerDisconnect"),
    ("AI hook", ROOT / "SrcServer/src/systems/gameplay/character/core/character.cpp", "companions::ApplyAI(this)"),
    ("Runtime config", ROOT / "Configs/Companion.ini", "CrystalItemCode=0"),
]

failed = []
for label, path, needle in checks:
    if not path.exists():
        failed.append(f"{label}: missing {path.relative_to(ROOT)}")
        continue
    text = path.read_text(encoding="latin1", errors="replace")
    if needle not in text:
        failed.append(f"{label}: missing marker {needle!r}")

# Safety contracts we explicitly depend on.
cpp = (ROOT / "SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp").read_text(errors="replace")
if "COMPANION_MAX_ACTIVE = 4" not in (ROOT / "SrcServer/src/systems/gameplay/companions/CompanionSystem.h").read_text(errors="replace"):
    failed.append("max active companion contract is not 4")
if "FindUserFromName" not in cpp:
    failed.append("online-character exclusion is missing")
if "rsCheckAccountChar" not in cpp:
    failed.append("account ownership validation is missing")

if failed:
    for failure in failed:
        print(f"[FAIL] {failure}")
    sys.exit(1)

print(f"companion system v1 validation: PASS ({len(checks)} integration markers + safety contracts)")
