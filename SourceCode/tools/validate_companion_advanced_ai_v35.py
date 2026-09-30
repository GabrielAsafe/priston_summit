#!/usr/bin/env python3
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
s=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp').read_text(errors='ignore')
errors=[]

def req(label, needle):
    if needle not in s: errors.append(f"{label}: missing {needle!r}")

for needle in ('lastProgressCheckTime', 'lastProgressX', 'lastProgressZ', 'stuckTicks'):
    req('stuck slot state', needle)
for needle in ('FindNearestThreatToParty', 'ThreatTargetsParty(threat, owner, state)',
               'TryReadThreat(candidate, threat)',
               'target = FindNearestThreatToParty(character, owner, *state)'):
    req('defensive threat scoring', needle)
for needle in ('const int innerRange = preferredRange > 0', '(range * 55) / 100',
               'const int step = std::max(32, std::min(120, range - innerRange))',
               'character->SetTargetPosi2(moveX, moveZ)', 'character->TargetMoveCount = 2'):
    req('preferred-range kite', needle)
for needle in ('(DWORD)(dwPlayServTime - managedSlot->lastProgressCheckTime) >= 1000U',
               'movedX * movedX + movedZ * movedZ < (8 * 8)',
               'managedSlot->stuckTicks >= 2', 'AI_STUCK_RECOVER',
               'character->AutoMoveStep = 0', 'character->SetNextTarget(0, 0, 0)'):
    req('stuck recovery', needle)

# Ensure zero preferred range remains the opt-out; the kite branch must be gated.
if 'innerRange > 0 && distance < innerRange * innerRange' not in s:
    errors.append('kite is not gated by explicit non-zero preferredRange')

if errors:
    print('Companion Advanced AI V35: FAIL')
    for e in errors: print('  -', e)
    raise SystemExit(1)
print('Companion Advanced AI V35: PASS (threat-aware Defensive + preferred-range kite + stuck recovery)')
