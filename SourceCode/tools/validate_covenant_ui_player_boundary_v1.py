#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
contract=(root/'Shared/gameplay/covenants/v2/CovenantUIIngress.h').read_text()
public_h=(root/'SrcServer/src/systems/gameplay/covenants/v2/CovenantUIHandler.h').read_text()
public_cpp=(root/'SrcServer/src/systems/gameplay/covenants/v2/CovenantUIHandler.cpp').read_text()
legacy=(root/'SrcServer/src/systems/gameplay/covenants/v2/legacy/LegacyCovenantUIIngressAdapter.cpp').read_text()
server=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text()
allow=(root/'tools/architecture/legacy_boundary_allowlist.txt').read_text()
checks={
 'green ingress contract':'class ICovenantUIIngress' in contract and 'PlayerId owner' in contract,
 'public handler player-id':'HandleCovenantUIAction(PlayerId owner' in public_h,
 'public surface legacy-clean':all(x not in public_h+public_cpp for x in ('rsPLAYINFO','smCHAR','smwsock.h','smd3d.h','character.h','onserver.h','FLOATNS')),
 'facade delegates amber':'ServerLegacyCovenantUIIngressAdapter().HandleAction(owner, packet)' in public_cpp,
 'amber resolves owner':'players_.Resolve(ownerId)' in legacy and 'rsPLAYINFO* owner' in legacy,
 'legacy semantics retained':all(x in legacy for x in ('CanSwitch(owner','ActivateServerCovenant','UseServerCovenantPower','SetServerCovenantArtisanMode','RefineServerCovenantTrophy','COVENANT_UI_HARVEST_MINING','COVENANT_UI_HARVEST_GATHERING')),
 'server authenticates player id':'ServerLegacyOnlinePlayerRegistry().IdOf(lpPlayInfo)' in server and 'RouteServerGameplayPacket' in server,
 'covenant ui allowlist debt removed':all(x not in allow for x in ('CovenantUIHandler.h | symbol:rsPLAYINFO','CovenantUIHandler.cpp | include:smwsock.h','CovenantUIHandler.cpp | include:smd3d.h','CovenantUIHandler.cpp | include:character.h','CovenantUIHandler.cpp | include:onserver.h','CovenantUIHandler.cpp | symbol:rsPLAYINFO')),
}
for k,v in checks.items(): print(('PASS' if v else 'FAIL')+': '+k)
if not all(checks.values()): sys.exit(1)
print('covenant UI player boundary v1: PASS (PlayerId ingress public; kernel/socket implementation isolated in AMBER)')
