#!/usr/bin/env python3
from pathlib import Path
import re
import sys

root = Path(__file__).resolve().parents[1]
path = root / 'Shared/server/legacy_runtime/OnSever_legacy_server.inl'
text = path.read_text(errors='replace')
active = re.sub(r'/\*.*?\*/', '', text, flags=re.S)
active = re.sub(r'//.*', '', active)

start = active.find('int rsProcessAttack_SkillHolyReflection(')
end = active.find('int rsClanUpdate(', start)
if start < 0 or end < 0:
    raise SystemExit('FAIL: missing HolyReflection function boundary')
body = active[start:end]

checks = {
    'HolyReflection delegates lethal life mutation': 'ServerLegacyLifeAdapter().ConsumeLethal(' in body,
    'HolyReflection preserves killer attribution': 'lpChar->lpExt2 = (void *)lpPlayInfo;' in body,
    'HolyReflection has no direct life subtraction': re.search(r'lpChar->smCharInfo\.Life\[0\]\s*-=', body) is None,
    'HolyReflection has no direct life zero assignment': re.search(r'lpChar->smCharInfo\.Life\[0\]\s*=\s*0', body) is None,
    'HolyReflection no longer owns death motion': 'SetMotionFromCode(CHRMOTION_STATE_DEAD)' not in body,
}
failed = [name for name, ok in checks.items() if not ok]
for name, ok in checks.items():
    print(('PASS: ' if ok else 'FAIL: ') + name)
if failed:
    sys.exit(1)
print('onserver holy reflection life authority v1: PASS')
