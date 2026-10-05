#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
s=(ROOT/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(encoding='utf-8')
a=s.index('int srAutoCharMain(smCHAR *lpChar)')
b=s.index('int srAutoPlayMain()',a)
block=s[a:b]
checks=[
 ('autochar clamp uses life authority','ServerLegacyLifeAdapter().ClampToValidRange' in block),
 ('negative direct clamp removed','if(lpChar->smCharInfo.Life[0] < 0) lpChar->smCharInfo.Life[0] = 0;' not in block),
 ('maximum direct clamp removed','if(lpChar->smCharInfo.Life[0] > lpChar->smCharInfo.Life[1]) lpChar->smCharInfo.Life[0] = lpChar->smCharInfo.Life[1];' not in block),
]
failed=[]
for n,ok in checks:
 print(('PASS' if ok else 'FAIL')+': '+n)
 if not ok: failed.append(n)
if failed: sys.exit(1)
print('autochar life clamp authority v1: PASS')
