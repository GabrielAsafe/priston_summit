#!/usr/bin/env python3
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
s=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp').read_text(errors='ignore')
errors=[]
def req(label, needle):
    if needle not in s: errors.append(f"{label}: missing {needle!r}")
for needle in ('MoveTowardInjuredAlly', 'percent < 70',
               'distance > (320LL * 320LL)', 'distance <= (600LL * 600LL)',
               'percent < bestPercent || (percent == bestPercent && distance < bestDistance)'):
    req('healer recipient selection', needle)
for needle in ('DisengageTarget(healer)', 'healer->SetTargetPosi2',
               'healer->TargetMoveCount = 2', 'healer->SetMotionFromCode(CHRMOTION_STATE_RUN)'):
    req('healer movement', needle)
for needle in ('HasCompanionHealingRole(static_cast<classes::PlayableClassId>(',
               'GetCompanionSkillPoint(managedSlot->skillRuntime, 1) > 0',
               'healAttempt.reason == COMPANION_SKILL_NO_TARGET', 'AI_HEAL_POSITION'):
    req('AI integration', needle)
# Preserve the existing authoritative in-range heal call.
req('heal executor retained', 'TryExecuteCompanionHeal(character, owner, party, partyCount')
if errors:
    print('Companion Advanced AI V38: FAIL')
    for e in errors: print('  -', e)
    raise SystemExit(1)
print('Companion Advanced AI V38: PASS (out-of-range injured ally approach + existing heal executor)')
