#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
s=(ROOT/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(encoding='utf-8')
a=s.index('if(lpChar->UseObject_VirtualLife)')
b=s.index('case OpCode::OPCODE_CLIENTINFO:', a)
block=s[a:b]
checks=[
 ('virtual-life restore uses life authority','ServerLegacyLifeAdapter().RestoreToMaximum' in block),
 ('virtual-life restore has no direct current-life max assignment','lpChar->smCharInfo.Life[0] = lpChar->smCharInfo.Life[1];' not in block),
 ('virtual-life meter logic preserved','lpChar->sObject_VirtualLife[0] += cnt;' in block),
]
failed=[]
for n,ok in checks:
 print(('PASS' if ok else 'FAIL')+': '+n)
 if not ok: failed.append(n)
if failed: sys.exit(1)
print('onserver virtual life restore authority v1: PASS')
