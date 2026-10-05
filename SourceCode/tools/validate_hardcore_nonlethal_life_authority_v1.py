#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
s=(ROOT/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(encoding='utf-8')
a=s.index('int rsSoD_CatchItem_SD201(')
b=s.index('\n\n\n', a)
# Function is short; terminate on its explicit return/brace instead of relying on next declaration.
b=s.index('\n\treturn TRUE;\n}', a)+len('\n\treturn TRUE;\n}')
block=s[a:b]
checks=[
 ('hardcore floor damage uses nonlethal life authority','ServerLegacyLifeAdapter().ConsumeNonLethal' in block),
 ('zero/negative recovery uses life authority','ServerLegacyLifeAdapter().EnsureMinimum' in block),
 ('direct life subtraction removed','smCharInfo.Life[0] -= Damage;' not in block),
 ('direct life floor write removed','smCharInfo.Life[0] = 1;' not in block),
 ('damage motion preserved','SetMotionFromCode(CHRMOTION_STATE_DAMAGE)' in block),
]
failed=[]
for name,ok in checks:
 print(('PASS' if ok else 'FAIL')+': '+name)
 if not ok: failed.append(name)
if failed: sys.exit(1)
print('hardcore nonlethal life authority v1: PASS')
