#!/usr/bin/env python3
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
store_h = (ROOT / "SrcServer/src/systems/gameplay/companions/CompanionProfileStore.h").read_text(errors="ignore")
store_cpp = (ROOT / "SrcServer/src/systems/gameplay/companions/CompanionProfileStore.cpp").read_text(errors="ignore")
system = (ROOT / "SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp").read_text(errors="ignore")
page = (ROOT / "SrcGame/src/systems/ui/v2/pages/CompanionsPage.cpp").read_text(errors="ignore")

checks = [
    ("profile API", store_h, "LoadCompanionProfile"),
    ("profile API", store_h, "SaveCompanionProfile"),
    ("separate sidecar root", store_cpp, "DataServer\\\\CompanionProfiles"),
    ("versioned disk format", store_cpp, "COMPANION_PROFILE_VERSION = 4"),
    ("v3 migration accepted", store_cpp, "COMPANION_PROFILE_SKILL_LOADOUT_VERSION = 3"),
    ("v2 migration accepted", store_cpp, "COMPANION_PROFILE_V2_VERSION = 2"),
    ("disk ABI retained", store_cpp, "sizeof(CompanionProfileDiskV1) == 84"),
    ("skill loadout persisted", store_cpp, "COMPANION_PROFILE_SKILL_LOADOUT_ACTIVE"),
    ("buff loadout persisted", store_cpp, "COMPANION_PROFILE_BUFF_LOADOUT_ACTIVE"),
    ("checksummed disk format", store_cpp, "ComputeChecksum"),
    ("atomic replace", store_cpp, "MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH"),
    ("identity collision guard", store_cpp, "disk.accountHash != HashIdentity(account)"),
    ("lazy persisted loadout load", system, "LoadCompanionProfile(state.owner->getAccount(), characterName, &profile)"),
    ("loadout save", system, "PersistLoadoutOverride"),
    ("persistence error surfaced", page, "Falha ao persistir o perfil do companion"),
]
errors=[]
for name,text,needle in checks:
    if needle not in text:
        errors.append(f"{name}: missing {needle}")

# DAT must remain read-only from the loadout path: no record save call in the
# equipment mutation functions.
for fn in ("ChangeEquipment", "ResetEquipmentInternal"):
    match = re.search(rf'int {fn}\(.*?\n\}}', system, re.S)
    if not match:
        errors.append(f"missing function {fn}")
    elif "rsSave" in match.group(0) or "rsAddOfflineCharacterExp" in match.group(0):
        errors.append(f"{fn}: must not persist through character DAT")

if errors:
    print("Companion profile store: FAIL")
    for error in errors:
        print("  -", error)
    raise SystemExit(1)
print("Companion profile store: PASS")
