#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
ids=(root/'Shared/presentation/v2/PresentationIds.h').read_text()
target=(root/'Shared/presentation/v2/PresentationTarget.h').read_text()
contract=(root/'Shared/presentation/v2/LightingService.h').read_text()
adapter=(root/'SrcGame/src/systems/presentation/v2/legacy/LegacyLightingAdapter.cpp').read_text()
consumer=(root/'SrcGame/src/systems/network/client/netplay.cpp').read_text()
start=consumer.index('case SKILL_PLAY_RESURRECTION:')
end=consumer.index('case SKILL_PLAY_VIRTUAL_LIFE:',start)
block=consumer[start:end]
checks=[
 ('light id is logical key', 'struct LightId' in ids and 'std::string key' in ids),
 ('shared presentation target is renderer free', 'struct PresentationTarget' in target and 'smCHAR' not in target and 'D3D' not in target),
 ('green lighting port exists', 'class ILightingService' in contract and 'PulseAt' in contract),
 ('adapter maps resurrection light key', 'light.character.resurrection_restart' in adapter),
 ('adapter preserves historical light parameters', '100, 100, 100, 0, 200, 10' in adapter),
 ('adapter owns dynamic light call', 'SetDynLight(' in adapter),
 ('resurrection uses lighting service', 'ClientLegacyLightingAdapter().PulseAt' in block and 'light.character.resurrection_restart' in block),
 ('resurrection no direct dyn light', 'SetDynLight(' not in block),
 ('resurrection presentation trio remains', 'ClientLegacyAnimationAdapter().Play' in block and 'ClientLegacyVisualEffectAdapter().PlayAt' in block and 'ClientLegacyAudioAdapter().PlayForEntity' in block),
]
failed=[n for n,ok in checks if not ok]
for n,ok in checks: print(('PASS: ' if ok else 'FAIL: ')+n)
if failed: sys.exit(1)
print('resurrection presentation lighting boundary: PASS')
