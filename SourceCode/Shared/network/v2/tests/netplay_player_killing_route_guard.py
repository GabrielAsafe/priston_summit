#!/usr/bin/env python3
from pathlib import Path

root = Path(__file__).resolve().parents[4]
cases = [
    (root / 'SrcGame/src/systems/network/client/netplay.cpp', 'ClientLegacyBackendChannelSender'),
    (root / 'SrcServer/src/systems/network/server_gameplay/netplay.cpp', 'ServerLegacyBackendChannelSender'),
]

for path, accessor in cases:
    text = path.read_text(encoding='utf-8', errors='replace')
    start = text.index('int PlayerKilling(DWORD dwObjectSerial)')
    end = text.index('int SendCraftItemToServer', start)
    body = text[start:end]
    if f'{accessor}().Send(' not in body:
        raise SystemExit(f'{path}: PlayerKilling does not use typed backend sender')
    if 'BackendChannel::User' not in body:
        raise SystemExit(f'{path}: PlayerKilling lost User-channel routing')
    if 'OPCODE_PLAYERKILLING' not in body or 'TransCommand.size = sizeof(smTRANS_COMMAND)' not in body:
        raise SystemExit(f'{path}: PlayerKilling packet contract changed')
    if 'smWsockUserServer->Send(' in body:
        raise SystemExit(f'{path}: PlayerKilling still performs direct User socket send')

client = cases[0][0].read_text(encoding='utf-8', errors='replace')
start = client.index('int PlayerKilling(DWORD dwObjectSerial)')
end = client.index('int SendCraftItemToServer', start)
body = client[start:end]
if body.index('cKill.handleDeath();') > body.index('ClientLegacyBackendChannelSender().Send('):
    raise SystemExit('client PlayerKilling changed death-side-effect ordering')

print('netplay PlayerKilling typed User route guard: PASS')
