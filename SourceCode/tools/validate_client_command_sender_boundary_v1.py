#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
contract=(root/'Shared/network/v2/ClientCommandSender.h').read_text()
adapter=(root/'SrcGame/src/systems/network/v2/legacy/LegacyClientCommandSender.cpp').read_text()
companion=(root/'SrcGame/src/systems/ui/v2/legacy/LegacyCompanionGateway.cpp').read_text()
covenant=(root/'SrcGame/src/systems/ui/v2/legacy/LegacyCovenantGateway.cpp').read_text()
runner=(root/'Shared/network/v2/tests/run_packet_sender_tests.sh').read_text()
checks=[
 ('green sender contract exists', 'class IClientCommandSender' in contract and 'PacketView' in contract),
 ('green sender has no winsock', 'smWINSOCK' not in contract and 'smWsockServer' not in contract),
 ('adapter owns gameplay socket send', 'smWsockServer->Send' in adapter),
 ('companion routes command through sender', 'ClientLegacyCommandSender' in companion and 'sender.Send(gameplay::network::PacketView' in companion),
 ('companion has no direct gameplay socket send', 'smWsockServer->Send' not in companion),
 ('covenant routes command through sender', 'ClientLegacyCommandSender' in covenant and 'sender.Send(gameplay::network::PacketView' in covenant),
 ('covenant has no direct gameplay socket send', 'smWsockServer->Send' not in covenant),
 ('covenant no longer includes winsock', 'transport/legacy/smwsock.h' not in covenant),
 ('client command sender smoke is in portable runner', 'client_command_sender_smoke.cpp' in runner and 'ClientCommandSender smoke: PASS' in runner),
]
failed=[label for label,ok in checks if not ok]
for label,ok in checks: print(('PASS: ' if ok else 'FAIL: ')+label)
if failed: sys.exit(1)
print('client command sender boundary: PASS')
