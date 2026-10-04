#!/usr/bin/env python3
from pathlib import Path
root = Path(__file__).resolve().parents[5]
text = (root / 'SrcServer/src/systems/gameplay/character/core/character.cpp').read_text(encoding='utf-8', errors='replace')
start = text.index('const gameplay::world::ActorAutoMovePlan autoMovePlan =')
end = text.index('ptNextTarget.x = 0;', start)
block = text[start:end]
for token in (
    'ActorNavigationPolicy::PlanAutoMove(smMonsterInfo.IQ, dist)',
    'AutoMoveStep = autoMovePlan.walkStepCount;',
    'if(autoMovePlan.preferRun)',
    'SetMotionFromCode(CHRMOTION_STATE_RUN)',
    'AutoMoveStep = autoMovePlan.runStepCount;',
    'SetMotionFromCode(CHRMOTION_STATE_WALK)',
):
    assert token in block, f'missing server auto-move projection: {token}'
for legacy in ('smMonsterInfo.IQ >= 6', 'dist < (64 * 8 * 64 * 8)', 'AutoMoveStep <<='):
    assert legacy not in block, f'legacy server auto-move authority remains: {legacy}'
print('smCHAR actor navigation guard: PASS')
