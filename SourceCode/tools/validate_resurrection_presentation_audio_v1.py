#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
contract=(root/'Shared/presentation/v2/AudioService.h').read_text()
adapter=(root/'SrcGame/src/systems/presentation/v2/legacy/LegacyAudioAdapter.cpp').read_text()
consumer=(root/'SrcGame/src/systems/network/client/netplay.cpp').read_text()
start=consumer.index('case SKILL_PLAY_RESURRECTION:')
end=consumer.index('case SKILL_PLAY_VIRTUAL_LIFE:',start)
block=consumer[start:end]
checks=[
 ('audio service supports entity cue', 'PlayForEntity(gameplay::EntityId' in contract),
 ('green audio contract is character free', 'smCHAR' not in contract and 'CharPlaySound' not in contract),
 ('adapter maps resurrection character cue', 'character.resurrection_restart' in adapter and 'CharPlaySound' in adapter),
 ('adapter resolves entity in AMBER', 'FindChrPlayer' in adapter),
 ('resurrection uses targeted audio service', 'ClientLegacyAudioAdapter().PlayForEntity' in block and 'character.resurrection_restart' in block),
 ('resurrection no direct character sound', 'CharPlaySound(' not in block),
 ('resurrection effect boundary remains', 'ClientLegacyVisualEffectAdapter().PlayAt' in block),
 ('resurrection animation boundary remains', 'ClientLegacyAnimationAdapter().Play' in block),
]
failed=[n for n,ok in checks if not ok]
for n,ok in checks: print(('PASS: ' if ok else 'FAIL: ')+n)
if failed: sys.exit(1)
print('resurrection presentation audio boundary: PASS')
