#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
s=(ROOT/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(encoding='utf-8')
a=s.index('int\trsEasy_CastleMode()\n{')
b=s.index('int rsCheck_BlessCastleClanSkill(',a)
block=s[a:b]
checks=[
 ('easy castle zeroes virtual-life meter','sObject_VirtualLife[0] = 0;' in block),
 ('easy castle depletes through life authority','ServerLegacyLifeAdapter().Deplete' in block),
 ('easy castle restores survival floor through life authority','ServerLegacyLifeAdapter().EnsureMinimum' in block),
 ('easy castle direct one-hp write removed','smCharInfo.Life[0] = 1;' not in block),
]
failed=[]
for n,ok in checks:
 print(('PASS' if ok else 'FAIL')+': '+n)
 if not ok: failed.append(n)
if failed: sys.exit(1)
print('easy castle life authority v1: PASS')
