#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
onserver=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='replace')
router=(root/'SrcServer/src/systems/network/v2/legacy/LegacyGameplayPacketRouter.cpp').read_text(errors='replace')
checks={
 'generic router before legacy switch': onserver.find('RouteServerGameplayPacket(') < onserver.find('switch(dwTransCode)') and onserver.find('RouteServerGameplayPacket(') >= 0,
 'unknown opcode falls through': 'RouteStatus::UnknownOpcode' in onserver,
 'handled exits common cleanup': 'goto quit;' in onserver[onserver.find('RouteServerGameplayPacket('):onserver.find('switch(dwTransCode)')],
 'no covenant feature case': 'case gameplay::covenants::protocol::OPCODE_COVENANT_UI_ACTION' not in onserver,
 'no companion feature case': 'case gameplay::companions::protocol::OPCODE_COMPANION_UI_ACTION' not in onserver,
 'registry owns feature opcodes': 'CovenantHandler' in router and 'CompanionHandler' in router,
}
failed=[k for k,v in checks.items() if not v]
if failed:
 print('onserver packet router freeze v1: FAIL: '+', '.join(failed), file=sys.stderr); sys.exit(1)
print('onserver packet router freeze v1: PASS')
