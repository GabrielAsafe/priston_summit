#!/usr/bin/env python3
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
s=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp').read_text(errors='ignore')
errors=[]
def req(label, needle):
    if needle not in s: errors.append(f"{label}: missing {needle!r}")

for needle in ('ScoreHostileForCompanion', 'FindBestHostile', 'candidate == currentTarget',
               'ThreatTargetsOwner(threat, owner)', 'threat.targetEntity == EntityId'):
    req('target scoring', needle)
for needle in ('score -= 40000', 'score -= 50000', 'score -= 35000'):
    req('bounded retention/threat weights', needle)
for needle in ('HasOtherCompanionTargeting', 'CombatFormationOffset', '{ -36, -36 }', '{  36,  36 }',
               'EngageTarget(character, target, preferredRange, formationSlot, combatSpacing)'):
    req('combat spacing', needle)
for needle in ('if(combatSpacing)', 'character->TragetTraceMode = FALSE',
               'character->SetTargetPosi2((target->pX >> FLOATNS) + offsetX'):
    req('offset chase', needle)
# Owner focus and balanced must still keep explicit owner target ahead of generic scoring.
if s.count('target = ownerTarget;') < 2:
    errors.append('owner target priority no longer explicit in owner-focus/balanced')
# Do not replace all legacy tracing: solo targeting retains the exact-target path.
req('solo legacy trace', 'character->TragetTraceMode = TRUE;')

if errors:
    print('Companion Advanced AI V36: FAIL')
    for e in errors: print('  -', e)
    raise SystemExit(1)
print('Companion Advanced AI V36: PASS (stable target scoring + shared-target combat spacing)')
