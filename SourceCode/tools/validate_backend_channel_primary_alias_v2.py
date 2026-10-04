#!/usr/bin/env python3
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
files=[
 ROOT/'SrcGame/src/systems/network/v2/legacy/LegacyBackendChannelSender.h',
 ROOT/'SrcGame/src/systems/network/v2/legacy/LegacyBackendChannelSender.cpp',
 ROOT/'SrcServer/src/systems/network/v2/legacy/LegacyBackendChannelSender.h',
 ROOT/'SrcServer/src/systems/network/v2/legacy/LegacyBackendChannelSender.cpp',
]
checks=[]
for p in files:
 s=p.read_text(errors='ignore')
 rel=p.relative_to(ROOT).as_posix()
 checks.append((f'{rel} exposes/implements primary-alias query','AvailableAndDistinctFromPrimary' in s))
for p in [files[1],files[3]]:
 s=p.read_text(errors='ignore'); rel=p.relative_to(ROOT).as_posix()
 checks += [
  (f'{rel} resolves backend socket before comparison','ResolveBackendSocket(channel)' in s),
  (f'{rel} compares against primary socket','socket != smWsockServer' in s),
 ]
for n,ok in checks: print(('PASS' if ok else 'FAIL')+': '+n)
if not all(ok for _,ok in checks): raise SystemExit(1)
print(f'backend channel primary alias v2: PASS ({len(checks)}/{len(checks)})')
