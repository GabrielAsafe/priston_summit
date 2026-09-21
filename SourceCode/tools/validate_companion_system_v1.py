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
    ("Runtime lifetime config", ROOT / "Configs/Companion.ini", "LifetimeMinutes=720"),
    ("Runtime flush config", ROOT / "Configs/Companion.ini", "ExpFlushSeconds=30"),
    ("Record header bridge", ROOT / "SrcServer/src/systems/persistence/character_record/record.h", "rsAddOfflineCharacterExp"),
    ("Crystal switch compile fix", ROOT / "Shared/server/legacy_runtime/OnSever_legacy_server.inl", "legacy unbraced switch case"),
    ("Companion include-order compile fix", ROOT / "SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp", 'systems/network/transport/legacy/smwsock.h'),
    ("Companion support include", ROOT / "SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp", 'core/gameplay/legacy_support/playsub.h'),
    ("Variable-sized account record read", ROOT / "Shared/persistence/character_record/record_legacy_server.inl", "fread(lpRecordData, 1, sizeof(TRANS_RECORD_DATA), fp)"),
    ("Variable-sized offline EXP read", ROOT / "Shared/persistence/character_record/record_legacy_server.inl", "fread(&recordData, 1, sizeof(TRANS_RECORD_DATA), fp)"),
    ("Record size bounded by bytes read", ROOT / "Shared/persistence/character_record/record_legacy_server.inl", "(size_t)lpRecordData->size > readBytes"),
    ("Direct companion slot spawn", ROOT / "Shared/server/legacy_runtime/OnSever_legacy_server.inl", "smCHAR *companion = srFindEmptyChar()"),
    ("Companion DAT-authoritative visual bridge", ROOT / "Shared/server/legacy_runtime/OnSever_legacy_server.inl", "The DataServer record is the authority for the companion's *network*"),
    ("Companion stable server-side pattern", ROOT / "Shared/server/legacy_runtime/OnSever_legacy_server.inl", "companion->SetLoadPattern(templateInfo->szModelName)"),
    ("Companion visibility bootstrap", ROOT / "Shared/server/legacy_runtime/OnSever_legacy_server.inl", "presenceGroup->code = OpCode::OPCODE_PLAYDATAGROUP"),
    ("Companion full info bootstrap", ROOT / "Shared/server/legacy_runtime/OnSever_legacy_server.inl", "companionInfo.code = OpCode::OPCODE_PLAYERINFO"),
    ("CLI active under Wine", ROOT / "Shared/server/legacy_runtime/OnSever_legacy_server.inl", "std::thread cmd(&CLI::run, CLI::getInstance(), &done)"),
    ("Full-map Enter close guard", ROOT / "SrcGame/src/platform/windows/application/Winmain.cpp", "g_BlockFullZoomCloseOnEnter = 1"),
    ("Full-map guard declaration", ROOT / "SrcGame/src/systems/ui/map/full_zoom/FullZoomMap.h", "extern int  g_BlockFullZoomCloseOnEnter"),
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

include_order = [
    '#include "systems/network/transport/legacy/smwsock.h"',
    '#include "systems/rendering/legacy_3d/smd3d.h"',
    '#include "systems/gameplay/character/core/character.h"',
]
positions = [cpp.find(token) for token in include_order]
if any(pos < 0 for pos in positions) or positions != sorted(positions):
    failed.append("legacy include-order compile fix is missing")

server_runtime = (ROOT / "Shared/server/legacy_runtime/OnSever_legacy_server.inl").read_text(encoding="latin1", errors="replace")
if "FIELD_STATE_VILLAGE" in server_runtime[server_runtime.find("OpenCharacterCompanionFromRecord"):server_runtime.find("OpenCharacterCompanionFromRecord") + 9000]:
    failed.append("companion spawn still blocks village maps")
if "SummitIsRunningUnderWine" in server_runtime:
    failed.append("Wine CLI suppression helper is still present")

if failed:
    for failure in failed:
        print(f"[FAIL] {failure}")
    sys.exit(1)

print(f"companion system v1 validation: PASS ({len(checks)} integration markers + safety contracts)")
