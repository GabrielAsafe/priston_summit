#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
adapter=(root/'SrcGame/src/systems/presentation/v2/legacy/LegacyVisualEffectAdapter.cpp').read_text()
consumer=(root/'SrcGame/src/systems/network/client/netplay.cpp').read_text()
historical=(root/'Shared/rendering/effects/legacy_ho/HoEffectLegacyCommon.h').read_text()
start=consumer.index('case SKILL_PLAY_RESURRECTION:')
end=consumer.index('case SKILL_PLAY_VIRTUAL_LIFE:',start)
block=consumer[start:end]
checks=[
 ('adapter maps resurrection effect key', 'effect.character.resurrection_restart' in adapter),
 ('adapter maps historical game-start id', 'kGameStartEffectIndex = 500' in adapter),
 ('historical effect id remains 500', '#define EFFECT_GAME_START1' in historical and '500' in historical),
 ('resurrection uses visual effect service', 'ClientLegacyVisualEffectAdapter().PlayAt' in block and 'effect.character.resurrection_restart' in block),
 ('resurrection no direct game-start effect', 'StartEffect(' not in block and 'EFFECT_GAME_START1' not in block),
 ('resurrection animation boundary remains', 'ClientLegacyAnimationAdapter().Play' in block),
]
failed=[n for n,ok in checks if not ok]
for n,ok in checks: print(('PASS: ' if ok else 'FAIL: ')+n)
if failed: sys.exit(1)
print('resurrection presentation effect boundary: PASS')
