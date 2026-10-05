#!/usr/bin/env python3
from pathlib import Path
import re
import sys

root = Path(__file__).resolve().parents[1]
path = root / 'SrcServer/src/systems/gameplay/combat/v2/legacy/LegacyLifeAdapter.cpp'
text = path.read_text(errors='replace')
start = text.find('smCHAR* ResolveEntity(EntityId target)')
end = text.find('\n}\n\nLifeCommandResult LegacyLifeAdapter::Restore', start)
if start < 0 or end < 0:
    raise SystemExit('FAIL: missing ResolveEntity boundary')
body = text[start:end]
checks = {
    'global entity resolver retained': 'srFindCharFromSerial(serial);' in body,
    'stage entity fallback scans all areas': 'for(int area = 0; area < StageAreaCount; ++area)' in body,
    'stage resolver overload used': 'srFindCharFromSerial(serial, area);' in body,
    'stage scan guarded by StageArea': 'if(StageArea)' in body,
    'unresolved entity fails closed': re.search(r'\breturn 0;', body) is not None,
}
failed = [name for name, ok in checks.items() if not ok]
for name, ok in checks.items():
    print(('PASS: ' if ok else 'FAIL: ') + name)
if failed:
    sys.exit(1)
print('life adapter entity resolution v1: PASS')
