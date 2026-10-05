#!/usr/bin/env python3
from pathlib import Path
import re
import sys

root = Path(__file__).resolve().parents[1]
path = root / 'Shared/server/legacy_runtime/OnSever_legacy_server.inl'
text = path.read_text(errors='replace')
active = re.sub(r'/\*.*?\*/', '', text, flags=re.S)
active = re.sub(r'//.*', '', active)

def function_body(signature: str, next_signature: str) -> str:
    start = active.find(signature)
    if start < 0:
        raise SystemExit('FAIL: missing function: ' + signature)
    end = active.find(next_signature, start + len(signature))
    if end < 0:
        end = len(active)
    return active[start:end]

healing = function_body('int\trsPlayHealing(smTRANS_COMMAND* lpTransCommand)', 'int rsPlayGrandHealing(')
clan_heal = function_body('int rsRecvYahooMotion(', 'smCHAR *rsFindAroundMonster(')

checks = {
    'server OnServer owns life adapter include': 'LegacyLifeAdapter.h' in text,
    'rsPlayHealing delegates restore': 'ServerLegacyLifeAdapter().Restore(' in healing,
    'rsPlayHealing has no direct life addition': re.search(r'lpChar->smCharInfo\.Life\[0\]\s*\+=', healing) is None,
    'rsPlayHealing has no direct life clamp assignment': re.search(r'lpChar->smCharInfo\.Life\[0\]\s*=\s*lpChar->smCharInfo\.Life\[1\]', healing) is None,
    'clan heal delegates restore': 'ServerLegacyLifeAdapter().Restore(' in clan_heal,
    'clan heal has no direct life addition': re.search(r'lpCharList\[cnt\]->smCharInfo\.Life\[0\]\s*\+=', clan_heal) is None,
    'clan heal has no direct life clamp assignment': re.search(r'lpCharList\[cnt\]->smCharInfo\.Life\[0\]\s*=\s*lpCharList\[cnt\]->smCharInfo\.Life\[1\]', clan_heal) is None,
}
failed = [name for name, ok in checks.items() if not ok]
for name, ok in checks.items():
    print(('PASS: ' if ok else 'FAIL: ') + name)
if failed:
    sys.exit(1)
print('onserver healing life authority v1: PASS (direct heal/clamp mutations retired)')
