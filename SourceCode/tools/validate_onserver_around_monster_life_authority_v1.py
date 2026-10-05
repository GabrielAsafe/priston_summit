#!/usr/bin/env python3
from pathlib import Path
import re
import sys

root = Path(__file__).resolve().parents[1]
path = root / 'Shared/server/legacy_runtime/OnSever_legacy_server.inl'
text = path.read_text(errors='replace')
active = re.sub(r'/\*.*?\*/', '', text, flags=re.S)
active = re.sub(r'//.*', '', active)

start = active.find('smCHAR *rsSkillDamage_AroundMonster(')
end = active.find('smCHAR *rsSkillMonster_AroundMonster(', start)
if start < 0 or end < 0:
    raise SystemExit('FAIL: missing around-monster damage function boundary')
body = active[start:end]

checks = {
    'target damage delegates lethal life mutation': body.count('ServerLegacyLifeAdapter().ConsumeLethal(') >= 2,
    'target result drives death branch': 'targetLifeResult.Ok() && targetLifeResult.after <= 0' in body,
    'master attribution preserved': 'lpTargetChar->lpExt2 = lpChar->lpMasterPlayInfo;' in body,
    'half-exp attribution preserved': 'lpTargetChar->smMonsterInfo.GetExp >>= 1;' in body,
    'source backlash uses life authority': 'sourceLife' in body and 'lpChar->dwObjectSerial' in body,
    'no direct target life subtraction': re.search(r'lpTargetChar->smCharInfo\.Life\[0\]\s*-=', body) is None,
    'no direct target life zero': re.search(r'lpTargetChar->smCharInfo\.Life\[0\]\s*=\s*0', body) is None,
    'no direct source life zero': re.search(r'lpChar->smCharInfo\.Life\[0\]\s*=\s*0', body) is None,
}
failed = [name for name, ok in checks.items() if not ok]
for name, ok in checks.items():
    print(('PASS: ' if ok else 'FAIL: ') + name)
if failed:
    sys.exit(1)
print('onserver around-monster life authority v1: PASS')
