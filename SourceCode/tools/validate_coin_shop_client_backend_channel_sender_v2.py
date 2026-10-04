#!/usr/bin/env python3
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
files=[
 ROOT/'SrcGame/src/systems/commerce/coin_shop/CoinShop.cpp',
 ROOT/'SrcGame/src/systems/commerce/coin_shop/CoinShop_T.cpp',
]
base=(ROOT/'tools/architecture/direct_socket_send_baseline.json').read_text(errors='ignore')
checks=[]
for path in files:
    src=path.read_text(errors='ignore')
    rel=path.relative_to(ROOT).as_posix()
    start=base.find(f'"{rel}"')
    frag=base[start:start+260]
    checks.extend([
      (f'{path.name} includes backend sender','LegacyBackendChannelSender.h' in src),
      (f'{path.name} has three data-channel sends',src.count('BackendChannel::Data') == 3),
      (f'{path.name} direct data sends retired','smWsockDataServer->Send' not in src),
      (f'{path.name} socket budget retired','"smWsockDataServer": 0' in frag),
    ])
for n,ok in checks: print(('PASS' if ok else 'FAIL')+': '+n)
if not all(ok for _,ok in checks): raise SystemExit(1)
print(f'coin shop client backend channel sender v2: PASS ({len(checks)}/{len(checks)})')
