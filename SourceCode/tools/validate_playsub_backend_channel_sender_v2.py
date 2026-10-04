#!/usr/bin/env python3
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
commons=[
 'Shared/gameplay/legacy_support/PlaySubInventoryIntegrityLegacyCommon.inl',
 'Shared/gameplay/legacy_support/PlaySubInventoryTimingLegacyCommon.inl',
 'Shared/gameplay/legacy_support/PlaySubStartupModelLegacyCommon.inl',
]
wrappers=[
 ('SrcGame/src/core/gameplay/legacy_support/playsub.cpp','ClientLegacyBackendChannelSender'),
 ('SrcServer/src/core/gameplay/legacy_support/playsub.cpp','ServerLegacyBackendChannelSender'),
]
base=(ROOT/'tools/architecture/direct_socket_send_baseline.json').read_text(errors='ignore')
checks=[]
for rel in commons:
 src=(ROOT/rel).read_text(errors='ignore'); i=base.find(f'"{rel}"'); frag=base[i:i+260]
 checks += [
  (f'{rel} has no direct Data socket','smWsockDataServer' not in src),
  (f'{rel} uses PlaySub backend bridge','LegacyPlaySub' in src),
  (f'{rel} Data budget retired','"smWsockDataServer": 0' in frag),
 ]
for rel,sender in wrappers:
 src=(ROOT/rel).read_text(errors='ignore')
 checks += [
  (f'{rel} provides Data availability bridge','LegacyPlaySubDataChannelAvailable' in src and sender in src),
  (f'{rel} provides Data send bridge','LegacyPlaySubSendDataPacket' in src and 'BackendChannel::Data' in src),
 ]
for n,ok in checks: print(('PASS' if ok else 'FAIL')+': '+n)
if not all(ok for _,ok in checks): raise SystemExit(1)
print(f'playsub backend channel sender v2: PASS ({len(checks)}/{len(checks)})')
