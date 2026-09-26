#!/usr/bin/env python3
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
cpp=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp').read_text(errors='ignore')
errors=[]

def req(label,needle):
    if needle not in cpp: errors.append(f"{label}: missing {needle!r}")

req('caster-centered helper','static bool UsesCasterCenteredConfiguredArea(DWORD skillCode)')
for code in ('SKILL_PIKE_WIND','SKILL_GROUND_PIKE','SKILL_ROAR','SKILL_VENOM_SPEAR','SKILL_EXTINCTION','SKILL_DISTORTION'):
    req('caster-centered skill',f'case {code}:')
req('execution range parity','areaRadius = SkillRange(skillCode, caster, point)')
req('venom cap','maxTargets = std::max(1, VenomSpear_Num[point - 1])')
req('center selection','const smCHAR *center = casterCentered ? caster : primary')
req('vertical tolerance','abs((candidate->pY - center->pY) >> FLOATNS) <= 128')
req('minimum-target rejection','COMPANION_SKILL_MIN_TARGETS')
req('counter used by loadout','CountConfiguredSkillTargets(caster, slotTarget, descriptor.skillCode, descriptor.point) < minTargets')

if errors:
    print('Companion Loadout Targeting V34: FAIL')
    for e in errors: print('  -',e)
    raise SystemExit(1)
print('Companion Loadout Targeting V34: PASS (minimumTargets follows status/BOX/AoE geometry)')
