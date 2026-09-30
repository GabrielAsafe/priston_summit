#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
router=(root/'SrcServer/src/systems/network/v2/legacy/LegacyGameplayPacketRouter.cpp').read_text(errors='replace')
onserver=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='replace')
checks=[
 ('hellsgate handler exists', 'class HellsGateJoinHandler final : public IProtocolHandler' in router),
 ('hellsgate opcode owned by router', 'OpCode::OPCODE_OPEN_HG' in router and 'router.Register(hellsGateJoin)' in router),
 ('authenticated player id forwarded', 'cHg.RequestJoin(player);' in router),
 ('legacy payload remains opaque', 'RouteStatus Handle(PlayerId player, const PacketView&)' in router),
 ('onserver no hellsgate case', 'case OpCode::OPCODE_OPEN_HG' not in onserver),
 ('generic router executes before switch', 'RouteServerGameplayPacket(' in onserver and onserver.index('RouteServerGameplayPacket(') < onserver.index('switch(dwTransCode)')),
]
failed=[name for name,ok in checks if not ok]
for name,ok in checks: print(('PASS: ' if ok else 'FAIL: ')+name)
if failed: sys.exit(1)
print('hellsgate packet ingress: PASS')
