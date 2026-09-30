#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[1]
contract=(root/'Shared/gameplay/pets/v2/PetRuntime.h').read_text()
adapter=(root/'SrcServer/src/systems/gameplay/pets/v2/legacy/LegacyPetRuntimeAdapter.cpp').read_text()
party=(root/'SrcServer/src/core/server/legacy_runtime/PartySever.cpp').read_text()
onserver=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text()
start=onserver.index('int rsSendAreaServerForceOrb')
end=onserver.index('int rsRecvAreaServerForceOrb', start)
forceorb=onserver[start:end]
checks=[
 ('green pet runtime snapshot','struct PetRuntimeSnapshot' in contract and 'Snapshot(PlayerId player) const' in contract),
 ('snapshot exposes phoenix/help state','PhoenixActive()' in contract and 'HelpActive()' in contract),
 ('adapter maps legacy pet state','player->dwTime_PrimeItem_PhenixPet' in adapter and 'player->dwTime_PrimeItem_HelpPet' in adapter),
 ('party exp consumes snapshot','ServerLegacyPetRuntimeAdapter().Snapshot' in party and 'petSnapshot.PhoenixActive()' in party),
 ('party exp no direct phoenix timer','dwTime_PrimeItem_PhenixPet' not in party),
 ('forceorb publishes phoenix unconditionally','rsSendAreaServer_PhenixPet(lpPlayInfo);' in forceorb and 'if(lpPlayInfo->dwTime_PrimeItem_PhenixPet' not in forceorb),
 ('forceorb publishes help unconditionally','rsSendAreaServer_HelpPet(lpPlayInfo);' in forceorb and 'if(lpPlayInfo->dwTime_PrimeItem_HelpPet' not in forceorb),
]
failed=[]
for name,ok in checks:
 print(('PASS' if ok else 'FAIL')+': '+name)
 if not ok: failed.append(name)
if failed: raise SystemExit(1)
print('pet status query boundary: PASS')
