#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
path = ROOT / 'SrcServer/src/systems/social/clan/customization/ClanCustom.cpp'
text = path.read_text(encoding='latin-1')
checks = [
    ('ClanCustom uses clan query adapter', 'ServerLegacyClanQueryAdapter().TryGetMembership' in text),
    ('ClanCustom resolves PlayerId through registry', 'ServerLegacyOnlinePlayerRegistry().IdOf(player)' in text),
    ('ClanCustom leader authorization uses snapshot mark', 'clanMembership.mark -1000000' in text and 'rsBlessCastle.dwMasterClan == (clanMembership.mark)' in text),
    ('ClanCustom tag lookup uses snapshot mark', "[MiconCnt]='%d'\", clanMembership.mark" in text),
    ('ClanCustom has no active direct player clan-mark read', 'Player->smCharInfo.ClassClan' not in '\n'.join(line.split('//',1)[0] for line in text.splitlines())),
]
failed=[]
for name,ok in checks:
    print(('PASS' if ok else 'FAIL')+': '+name)
    if not ok: failed.append(name)
if failed: sys.exit(1)
print('clan custom query boundary v1: PASS')
