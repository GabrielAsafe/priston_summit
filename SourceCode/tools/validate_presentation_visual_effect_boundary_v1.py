#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
ids=(root/'Shared/presentation/v2/PresentationIds.h').read_text()
contract=(root/'Shared/presentation/v2/VisualEffectService.h').read_text()
target=(root/'Shared/presentation/v2/PresentationTarget.h').read_text()
adapter=(root/'SrcGame/src/systems/presentation/v2/legacy/LegacyVisualEffectAdapter.cpp').read_text()
consumer=(root/'SrcGame/src/systems/gameplay/character/status_ui/sinCharStatus.cpp').read_text()
legacy_catalog=(root/'Shared/rendering/effects/legacy_ho/HoEffectLegacyCommon.h').read_text()
checks=[
 ('effect id is logical key', 'struct EffectId' in ids and 'std::string key' in ids),
 ('presentation target is renderer free', 'struct PresentationTarget' in target and 'int x;' in target and 'smCHAR' not in target and 'D3D' not in target),
 ('green visual effect port exists', 'class IVisualEffectService' in contract and 'PlayAt' in contract),
 ('adapter maps reset effect', 'effect.ui.stat_reset' in adapter and 'kAgingEffectIndex = 5001' in adapter),
 ('adapter index matches historical catalog', '#define EFFECT_AGING' in legacy_catalog and '5001' in legacy_catalog),
 ('adapter owns StartEffect call', 'StartEffect(target.x, target.y, target.z, effectIndex, 1)' in adapter),
 ('consumer uses effect service', 'ClientLegacyVisualEffectAdapter().PlayAt' in consumer and 'EffectId("effect.ui.stat_reset")' in consumer),
 ('migrated consumer no direct aging effect call', 'StartEffect(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, EFFECT_AGING)' not in consumer),
]
failed=[name for name,ok in checks if not ok]
for name,ok in checks: print(('PASS: ' if ok else 'FAIL: ')+name)
if failed: sys.exit(1)
print('presentation visual effect boundary: PASS')
