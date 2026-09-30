#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[1]
contract=(root/'Shared/gameplay/pets/v2/PetRuntime.h').read_text()
adapter=(root/'SrcServer/src/systems/gameplay/pets/v2/legacy/LegacyPetRuntimeAdapter.cpp').read_text()
onserver=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text()
checks=[
 ('publish port exists','PublishAreaSync' in contract and 'enum class PetPublishStatus' in contract),
 ('adapter keeps 17s cadence','1000 * 17' in adapter),
 ('adapter keeps pcbang entitlement','rsHasPCBangGameplayEntitlement' in adapter),
 ('adapter keeps shared phoenix cadence','PetRuntimeKind::Phoenix' in adapter and '&player->dwPCBang_Pet_NextSetTime' in adapter),
 ('adapter keeps help cadence','PetRuntimeKind::Help' in adapter and '&player->dwHelp_Pet_NextSetTime' in adapter),
 ('adapter preserves mask','dwObjectSerial * (player->dwCode_Name << 3) ^ codeKey' in adapter),
 ('adapter preserves checksum','value + *nextSetTime + command.code' in adapter),
 ('adapter preserves help route','rsSendAGameServer_PrimeItem3' in adapter),
 ('adapter preserves other route','rsSendAGameServer(player->lpsmSock' in adapter),
 ('legacy wrappers delegate',onserver.count('ServerLegacyPetRuntimeAdapter().PublishAreaSync') >= 3),
 ('legacy send functions no wire assembly', all(token not in ''.join([
     onserver[onserver.index('int rsSendAreaServer_PCBangPet'):onserver.index('int rsSendAreaServer_PhenixPet')],
     onserver[onserver.index('int rsSendAreaServer_PhenixPet'):onserver.index('int rsRecvAreaServer_PhenixPet')],
     onserver[onserver.index('int rsSendAreaServer_HelpPet'):onserver.index('int rsRecvAreaServer_HelpPet')],
 ]) for token in ['dwMaskKey','1000 * 17','rsSendAGameServer_PrimeItem3(lpPlayInfo','rsSendAGameServer(lpPlayInfo'])),
]
failed=[]
for name,ok in checks:
 print(('PASS' if ok else 'FAIL')+': '+name)
 if not ok: failed.append(name)
if failed: raise SystemExit(1)
print('pet publish boundary: PASS')
