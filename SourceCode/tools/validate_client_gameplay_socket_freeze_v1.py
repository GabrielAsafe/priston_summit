#!/usr/bin/env python3
from pathlib import Path
import re
import sys

root = Path(__file__).resolve().parents[1]
client_root = root / 'SrcGame/src'
pattern = re.compile(r'\bsmWsockServer\s*->\s*Send\s*\(')

# Explicit kernel/backend ownership. These are not migration debt hidden in a
# generic allowlist: netplay/Damage/legacy_sin are frozen RED, while the V2
# sender is the single AMBER entrypoint used by modernized feature paths.
allowed = {
    'SrcGame/src/systems/network/v2/legacy/LegacyClientCommandSender.cpp': 'AMBER command sender adapter',
}

hits = {}
for path in client_root.rglob('*'):
    if not path.is_file() or path.suffix.lower() not in {'.cpp', '.cc', '.cxx', '.h', '.hpp'}:
        continue
    text = path.read_text(errors='replace')
    count = len(pattern.findall(text))
    if count:
        rel = path.relative_to(root).as_posix()
        hits[rel] = count

unexpected = {rel: count for rel, count in hits.items() if rel not in allowed}
missing_adapter = 'SrcGame/src/systems/network/v2/legacy/LegacyClientCommandSender.cpp' not in hits

for rel in sorted(hits):
    ownership = allowed.get(rel, 'UNEXPECTED')
    print(f'{rel}: direct_gameplay_sends={hits[rel]} ownership={ownership}')

if unexpected:
    for rel, count in sorted(unexpected.items()):
        print(f'FAIL: unclassified direct gameplay socket send: {rel} ({count})', file=sys.stderr)
    sys.exit(1)
if missing_adapter:
    print('FAIL: AMBER LegacyClientCommandSender no longer owns a gameplay socket send', file=sys.stderr)
    sys.exit(1)

feature_files = [
    'SrcGame/src/systems/ui/v2/legacy/LegacyCompanionGateway.cpp',
    'SrcGame/src/systems/ui/v2/legacy/LegacyCovenantGateway.cpp',
    'SrcGame/src/systems/gameplay/events/hells_gate/cHellsGate.cpp',
    'SrcGame/src/systems/social/party/ui/cPartyWindow.cpp',
    'SrcGame/src/systems/commerce/restoration/cRestaureWindow.cpp',
    'SrcGame/src/systems/gameplay/loot/roll/CRollWindow.cpp',
    'SrcGame/src/systems/commerce/premium/HaPremiumItem.cpp',
]
for rel in feature_files:
    text = (root / rel).read_text(errors='replace')
    if pattern.search(text):
        print(f'FAIL: migrated feature regained direct gameplay socket send: {rel}', file=sys.stderr)
        sys.exit(1)

print(f'client gameplay socket freeze v1: PASS (direct-send files={len(hits)}, unexpected=0)')
