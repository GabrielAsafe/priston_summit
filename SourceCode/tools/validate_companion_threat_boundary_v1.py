#!/usr/bin/env python3
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
contract = (ROOT/'Shared/gameplay/combat/v2/ThreatService.h').read_text(errors='ignore')
adapter_h = (ROOT/'SrcServer/src/systems/gameplay/combat/v2/legacy/LegacyThreatService.h').read_text(errors='ignore')
adapter_cpp = (ROOT/'SrcServer/src/systems/gameplay/combat/v2/legacy/LegacyThreatService.cpp').read_text(errors='ignore')
companion = (ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp').read_text(errors='ignore')
errors=[]

def req(label,text,needle):
    if needle not in text:
        errors.append(f"{label}: missing {needle!r}")

for n in ('struct ThreatSnapshot', 'PlayerId targetPlayer', 'EntityId targetEntity', 'class IThreatService', 'TryGetThreat'):
    req('GREEN threat contract', contract, n)
for n in ('class LegacyThreatService final : public IThreatService', 'ServerLegacyThreatService'):
    req('legacy threat adapter', adapter_h, n)
for n in ('resolved->lpTargetPlayInfo', 'resolved->lpTargetChar', 'snapshot.targetPlayer', 'snapshot.targetEntity'):
    req('legacy threat translation', adapter_cpp, n)
for n in ('TryReadThreat', 'ThreatTargetsParty', 'ThreatTargetsOwner', 'ThreatTargetsCompanion',
          'combat::ThreatSnapshot threat', 'TryReadThreat(candidate, threat)'):
    req('companion threat consumption', companion, n)
for forbidden in ('candidate->lpTargetPlayInfo == owner',
                  'state.slots[slot].entity == candidate->lpTargetChar',
                  'candidate->lpTargetChar == companion',
                  'currentTarget->lpTargetChar == character || currentTarget->lpTargetPlayInfo == owner'):
    if forbidden in companion:
        errors.append(f'companion still owns direct threat read: {forbidden!r}')

if errors:
    print('companion threat boundary v1: FAIL')
    for e in errors:
        print('  -', e)
    raise SystemExit(1)
print('companion threat boundary v1: PASS')
