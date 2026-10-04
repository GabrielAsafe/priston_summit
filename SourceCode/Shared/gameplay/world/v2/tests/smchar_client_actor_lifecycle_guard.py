#!/usr/bin/env python3
from pathlib import Path
root = Path(__file__).resolve().parents[5]
text = (root / 'Shared/gameplay/character/character_legacy_client.inl').read_text(encoding='utf-8', errors='replace')
assert '#include "gameplay/world/v2/ActorLifecycleService.h"' in text
main_start = text.index('int smCHAR::Main()')
case10_start = text.index('\n\tcase 10:', main_start)
case12_start = text.index('\n\tcase 12:', case10_start)
case99_start = text.index('\n\tcase 99:', case12_start)
case10 = text[case10_start:case12_start]
case12 = text[case12_start:case99_start]

for token in (
    'ActorLifecycleService::ResolveAnimationCompletion(',
    'motionCompletion.nextFrame',
    'ActorAnimationCompletionAction::HoldFinalFrame',
    'ActorAnimationCompletionAction::ReturnToStand',
):
    assert token in case10, f'missing case10 lifecycle projection: {token}'
assert 'frame = (MotionInfo->StartFrame * 160) + (frame - MotionInfo->EndFrame * 160);' not in case10

assert case12.count('ActorLifecycleService::AdvanceOneShot(') >= 2, 'warp lifecycle not mirrored in both case12 branches'
assert case12.count('warpAdvance.nextFrame') >= 2
assert case12.count('warpAdvance.finished') >= 2
assert 'ActorLifecycleService::ResolveAnimationCompletion(' in case12, 'case12 fallback completion still inline'
assert 'Close();' in case12 and 'SavePlayBuff();' in case12, 'warp side effects moved/lost'
print('smCHAR client actor lifecycle guard: PASS')
