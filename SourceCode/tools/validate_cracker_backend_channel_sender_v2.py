#!/usr/bin/env python3
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
common=(ROOT/'Shared/security/integrity/CrackerLegacyCommon.inl').read_text(errors='ignore')
client=(ROOT/'SrcGame/src/systems/security/legacy/process_integrity/cracker.cpp').read_text(errors='ignore')
server=(ROOT/'SrcServer/src/systems/security/legacy/process_integrity/cracker.cpp').read_text(errors='ignore')
base=(ROOT/'tools/architecture/direct_socket_send_baseline.json').read_text(errors='ignore')
rel='Shared/security/integrity/CrackerLegacyCommon.inl'; i=base.find(f'"{rel}"'); frag=base[i:i+260]
checks=[
 ('shared cracker no direct Data socket','smWsockDataServer' not in common),
 ('shared cracker checks target bridge','LegacyCrackerDataChannelAvailable()' in common),
 ('shared cracker sends through target bridge','LegacyCrackerSendDataPacket(' in common),
 ('client wrapper uses backend sender','ClientLegacyBackendChannelSender' in client and 'BackendChannel::Data' in client),
 ('server wrapper uses backend sender','ServerLegacyBackendChannelSender' in server and 'BackendChannel::Data' in server),
 ('shared cracker Data budget retired','"smWsockDataServer": 0' in frag),
]
for n,ok in checks: print(('PASS' if ok else 'FAIL')+': '+n)
if not all(ok for _,ok in checks): raise SystemExit(1)
print(f'cracker backend channel sender v2: PASS ({len(checks)}/{len(checks)})')
