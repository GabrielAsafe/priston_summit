#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[1]
contract=(root/'Shared/network/v2/ConnectionQuery.h').read_text()
adapter=(root/'SrcServer/src/systems/network/v2/legacy/LegacyConnectionQuery.cpp').read_text()
checks=[
 ('snapshot exposes remote address','std::string remoteAddress' in contract),
 ('contract remains socket free','smWINSOCK' not in contract and 'szIPAddr' not in contract),
 ('adapter translates legacy ip','player->lpsmSock->szIPAddr' in adapter and 'snapshot.remoteAddress' in adapter),
 ('address only on connected socket','snapshot.connected && player->lpsmSock->szIPAddr' in adapter),
]
failed=[]
for name,ok in checks:
 print(('PASS' if ok else 'FAIL')+': '+name)
 if not ok: failed.append(name)
if failed: raise SystemExit(1)
print('connection remote address boundary: PASS')
