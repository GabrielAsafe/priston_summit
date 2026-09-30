#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[1]

def function_body(text, signature):
 start=text.index(signature)
 brace=text.index('{', start)
 depth=0
 for i in range(brace, len(text)):
  if text[i]=='{': depth += 1
  elif text[i]=='}':
   depth -= 1
   if depth==0: return text[brace:i+1]
 raise RuntimeError('unterminated function '+signature)
contract=(root/'Shared/gameplay/events/v2/WorldEventAdmission.h').read_text()
adapter=(root/'SrcServer/src/systems/gameplay/events/v2/legacy/LegacyWorldEventAdmissionAdapter.cpp').read_text()
hg=(root/'SrcServer/src/systems/gameplay/events/hells_gate/cHellsGate.cpp').read_text()
h=(root/'SrcServer/src/systems/gameplay/events/hells_gate/cHellsGate.h').read_text()
request_join=function_body(hg, 'void cHELLSGATE::RequestJoin(gameplay::PlayerId playerId)')
send_open=function_body(hg, 'void cHELLSGATE::SendOpenNpc(gameplay::PlayerId player)')
onserver=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text()
router=(root/'SrcServer/src/systems/network/v2/legacy/LegacyGameplayPacketRouter.cpp').read_text()
if 'case OpCode::OPCODE_OPEN_HG:' in onserver:
 start=onserver.index('case OpCode::OPCODE_OPEN_HG:')
 end=onserver.index('case smTRANSCODE_OPEN_RANK:', start)
 block=onserver[start:end]
else:
 block=''
checks=[
 ('green admission port','class IWorldEventAdmissionService' in contract and 'WorldEventAdmissionRequest' in contract),
 ('adapter uses connection query','connections_.TryGet' in adapter and 'remoteAddress' in adapter),
 ('adapter uses player query','players_.TryGet' in adapter and 'position.map' in adapter),
 ('adapter preserves ordered online scan','onlinePlayers_.OnlineHandles()' in adapter),
 ('adapter preserves first matching ip break semantics','Preserve the legacy loop exactly' in adapter and 'return otherPlayer.position.map == request.occupiedMap' in adapter),
 ('hellsgate exposes request join','RequestJoin(gameplay::PlayerId player)' in h),
 ('request join enforces map 23 rule','WorldEventAdmissionRequest(' in request_join and 'playerId, gameplay::MapId(23u), true' in request_join),
 ('open npc does not own join admission','WorldEventAdmissionRequest(' not in send_open),
 ('hellsgate preserves duplicate message','HG> M\\341ximo 1 personagem por pessoa!' in hg),
 ('join ingress delegates through modern boundary', (('cHg.RequestJoin(' in block) or ('HellsGateJoinHandler' in router and 'cHg.RequestJoin(player)' in router)) and 'rsPlayInfo[' not in block and 'szIPAddr' not in block),
 ('join ingress no raw duplicate socket send','lpsmSock->Send' not in block),
]
failed=[]
for name,ok in checks:
 print(('PASS' if ok else 'FAIL')+': '+name)
 if not ok: failed.append(name)
if failed: raise SystemExit(1)
print('world event admission boundary: PASS')
