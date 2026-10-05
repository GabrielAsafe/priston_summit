#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
s=(ROOT/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(encoding='utf-8')
start=s.index('static void KillT5ArenaUnits(')
end=s.index('static void SendT5ArenaWarp',start)
kill=s[start:end]
start2=s.index('static bool SpawnT5ArenaWave(')
end2=s.index('static void UpdateT5ArenaTargets',start2)
spawn=s[start2:end2]
checks=[
 ('arena cleanup uses life authority','ServerLegacyLifeAdapter().ConsumeLethal' in kill),
 ('arena cleanup has no direct lethal write','lpChar->smCharInfo.Life[0] = 0' not in kill),
 ('failed spawn cleanup uses life authority','ServerLegacyLifeAdapter().ConsumeLethal' in spawn),
 ('failed spawn cleanup has no direct lethal write','lpHopy->smCharInfo.Life[0] = 0' not in spawn),
 ('arena spawn still owns initial HP construction','lpHopy->smCharInfo.Life[0] = hp;' in spawn and 'lpHopy->smCharInfo.Life[1] = hp;' in spawn),
]
failed=[]
for n,ok in checks:
 print(('PASS' if ok else 'FAIL')+': '+n)
 if not ok: failed.append(n)
if failed: sys.exit(1)
print('t5 arena life authority v1: PASS')
