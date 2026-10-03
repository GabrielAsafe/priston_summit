#!/usr/bin/env python3
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[1]
onserver = (root / 'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='replace')
netplay = (root / 'SrcGame/src/systems/network/client/netplay.cpp').read_text(errors='replace')
client_router = (root / 'SrcGame/src/systems/network/v2/legacy/LegacyClientGameplayPacketRouter.h').read_text(errors='replace')
chat_adapter = (root / 'SrcServer/src/systems/network/v2/legacy/LegacyModernChatIngressAdapter.inl').read_text(errors='replace')

failures = []

for literal in ('"/companion"', '"/covenant"', '"/mine"', '"/gather"', '"/resources"'):
    if literal in onserver:
        failures.append(f'OnServer still owns modern chat ingress {literal}')

for marker in ('OPCODE_COMPANION_UI_STATE', 'OPCODE_COMPANION_RUNTIME_STATE',
               'OPCODE_COMPANION_VISUAL_STATE', 'OPCODE_COVENANT_UI_STATE'):
    if marker in netplay:
        failures.append(f'client netplay still owns modern receive ingress {marker}')
    if marker not in client_router:
        failures.append(f'client AMBER router missing {marker}')

if 'RouteClientGameplayPacket(' not in netplay:
    failures.append('client netplay does not delegate to AMBER gameplay router')
if 'RouteModernChatIngress(' not in onserver:
    failures.append('OnServer does not delegate modern chat commands to AMBER adapter')

for literal in ('"/companion"', '"/covenant"', '"/mine"', '"/gather"', '"/resources"'):
    if literal not in chat_adapter:
        failures.append(f'AMBER chat adapter missing compatibility ingress {literal}')

if failures:
    for failure in failures:
        print('FAIL: ' + failure, file=sys.stderr)
    sys.exit(1)
print('red ingress retired v2: PASS (OnServer modern chat ingress=0, client netplay modern feature cases=0)')
