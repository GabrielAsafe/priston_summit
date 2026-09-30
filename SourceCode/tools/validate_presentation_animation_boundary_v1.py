#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
ids=(root/'Shared/presentation/v2/PresentationIds.h').read_text()
contract=(root/'Shared/presentation/v2/AnimationService.h').read_text()
adapter=(root/'SrcGame/src/systems/presentation/v2/legacy/LegacyAnimationAdapter.cpp').read_text()
consumer=(root/'SrcGame/src/systems/network/client/netplay.cpp').read_text()
start=consumer.index('case SKILL_PLAY_RESURRECTION:')
end=consumer.index('case SKILL_PLAY_VIRTUAL_LIFE:', start)
block=consumer[start:end]
checks=[
 ('animation id is logical key', 'struct AnimationId' in ids and 'std::string key' in ids),
 ('green animation port exists', 'class IAnimationService' in contract and 'gameplay::EntityId' in contract),
 ('green animation port is character free', 'smCHAR' not in contract and 'CHRMOTION_' not in contract),
 ('adapter maps restart key', 'character.restart' in adapter and 'CHRMOTION_STATE_RESTART' in adapter),
 ('adapter owns legacy character lookup', 'FindChrPlayer' in adapter and 'SetMotionFromCode' in adapter),
 ('resurrection uses animation service', 'ClientLegacyAnimationAdapter().Play' in block and 'AnimationId("character.restart")' in block),
 ('resurrection no direct restart motion', 'SetMotionFromCode(CHRMOTION_STATE_RESTART)' not in block),
]
failed=[n for n,ok in checks if not ok]
for n,ok in checks: print(('PASS: ' if ok else 'FAIL: ')+n)
if failed: sys.exit(1)
print('presentation animation boundary: PASS')
