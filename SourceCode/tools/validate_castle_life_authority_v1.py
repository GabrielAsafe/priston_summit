#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
s=(ROOT/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(encoding='utf-8')
a=s.index('int rsMain_CombatCastle()\n{')
b=s.index('int\trsEasy_CastleMode()\n{',a)
castle=s[a:b]
checks=[
 ('valhalla tower restore uses life authority','ServerLegacyLifeAdapter().RestoreToMaximum' in castle),
 ('castle end has lethal authority','ServerLegacyLifeAdapter().ConsumeLethal' in castle),
 ('valhalla direct current-life restore removed','lpChar_ValhallaTower->smCharInfo.Life[0] = lpChar_ValhallaTower->smCharInfo.Life[1];' not in castle),
 ('castle actor direct max restore removed','chrAutoPlayer[cnt].smCharInfo.Life[0] = chrAutoPlayer[cnt].smCharInfo.Life[1];' not in castle),
 ('castle actor direct lethal write removed','chrAutoPlayer[cnt].smCharInfo.Life[0] = 0;' not in castle),
 ('defence tower stand motion preserved','SetMotionFromCode(CHRMOTION_STATE_STAND)' in castle),
]
failed=[]
for n,ok in checks:
 print(('PASS' if ok else 'FAIL')+': '+n)
 if not ok: failed.append(n)
if failed: sys.exit(1)
print('castle life authority v1: PASS')
