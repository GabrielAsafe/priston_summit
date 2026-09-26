#!/usr/bin/env python3
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]

def read(rel):
    return (ROOT / rel).read_text(encoding="utf-8", errors="replace")

def require(cond, msg):
    if not cond:
        print(f"FAIL: {msg}")
        sys.exit(1)
    print(f"PASS: {msg}")

catalog = read("Shared/gameplay/skills/v2/content/LegacySkillCatalog.cpp")
skill_sub = read("Shared/gameplay/skills/SkillSub_legacy_client.inl")
onserver = read("Shared/server/legacy_runtime/OnSever_legacy_server.inl")
temps = read("Shared/gameplay/skills/Tempskron_legacy_client.inl")
morayion = read("Shared/gameplay/skills/Morayion_legacy_client.inl")
legacy_funcs = read("Shared/gameplay/skills/TempskronLegacySkillFunctions.inl")

require(
    '"Sinister Spin", 0xE4u, 0x16u' in catalog,
    "Sinister Spin preserves T54 network code and aliases only presentation to Tornado",
)
require(
    re.search(r"dm_SendRangeDamage\([^;]*SummitSkillEncodedNetworkPlayCode\(SKILL_T5_TWISTER,\s*lpSkill->Point\)\s*\);", skill_sub, re.S) is not None,
    "Sinister Spin damage packet uses T54 network code rather than presentation alias",
)

require("static void SummitReleaseTargetsForVanish" in onserver,
        "Vanish has an explicit server-side acquired-target release bridge")
require("SummitReleaseTargetsForVanish(lpPlayInfo);" in onserver,
        "Vanish invokes target release when the server accepts the cast")
for boss in ("snCHAR_SOUND_BABEL", "snCHAR_SOUND_DEATHKNIGHT", "snCHAR_SOUND_KELVEZU", "snCHAR_SOUND_MOKOVA"):
    require(boss in onserver, f"Vanish preserves legacy immunity for {boss}")
require("lpMonster->dwCompulsionTime > dwPlayServTime" in onserver and
        "lpMonster->lpCompulsionPlayInfo == lpPlayInfo" in onserver,
        "Vanish does not override active Compulsion forced targeting")
require("lpMonster->lpTargetPlayInfo = 0;" in onserver and
        "lpMonster->dwTargetLockTime = 0;" in onserver and
        "lpMonster->TargetMoveCount = 0;" in onserver,
        "Vanish clears existing normal-monster target/lock/movement state")

# Timed active T5 buffs should enter ContinueSkill. True passives intentionally do not.
for fn, source in (
    ("F_Hardening_Shield", legacy_funcs),
    ("F_Ruthless_Of_Zecram", legacy_funcs),
    ("F_Pike_T5_Focus", temps),
    ("F_Pike_T5_Death_Master", temps),
    ("F_Archer_T5_Lethal_Sight", temps),
    ("F_Priestess_T5_Consecration", morayion),
    ("F_Magician_T5_Wizard_Trance", morayion),
    ("F_Magician_T5_Stone_Skin", morayion),
    ("F_Knight_T5_Holy_Conviction", morayion),
    ("F_Knight_T5_Glorious_Shield", morayion),
):
    m = re.search(rf"int\s+{re.escape(fn)}\s*\([^)]*\)\s*\{{(.*?)\n\}}", source, re.S)
    require(m is not None and "sinContinueSkillSet(pUseSkill);" in m.group(1),
            f"{fn} registers a timed ContinueSkill entry")

require('"Survival Instinct", 0x00u, 0x00u' in catalog,
        "Fighter Survival Instinct remains a true passive (no cast/timer)")
require('"Golden Apple", 0x00u, 0x00u' in catalog,
        "Atalanta Golden Apple remains a true passive (no cast/timer)")

print("RESULT=PASS")
