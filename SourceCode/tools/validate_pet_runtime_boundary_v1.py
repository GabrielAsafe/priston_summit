#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[1]
contract=(root/'Shared/gameplay/pets/v2/PetRuntime.h').read_text()
adapter=(root/'SrcServer/src/systems/gameplay/pets/v2/legacy/LegacyPetRuntimeAdapter.cpp').read_text()
onserver=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text()
pos=onserver.index('case OpCode::OPCODE_PCBANG_PET:')
end=onserver.index('case OpCode::OPCODE_QUEST_COMMAND:', pos)
block=onserver[pos:end]
checks=[
 ('green pet runtime port','class IPetRuntime' in contract and 'ApplyAreaSync' in contract),
 ('green neutral pet sync payload','struct PetAreaSync' in contract and 'std::uint32_t w' in contract),
 ('three pet kinds','PcBang' in contract and 'Phoenix' in contract and 'Help' in contract),
 ('adapter preserves legacy mask','dwObjectSerial' in adapter and 'dwCode_Name' in adapter and '^ sync.e' in adapter),
 ('adapter validates checksum','w + l + sync.opcode' in adapter),
 ('adapter applies pcbang value','player->Bl_RNo' in adapter),
 ('adapter applies phoenix timer','dwTime_PrimeItem_PhenixPet' in adapter),
 ('adapter applies help timer','dwTime_PrimeItem_HelpPet' in adapter),
 ('onserver delegates pet sync','ServerLegacyPetRuntimeAdapter().ApplyAreaSync' in block),
 ('onserver authenticates player','ServerLegacyOnlinePlayerRegistry().IdOf(lpPlayInfo)' in block),
 ('onserver preserves all wire fields', all(x in block for x in ['WParam','LParam','SParam','EParam'])),
 ('onserver no direct old pet recv', all(x not in block for x in ['rsRecvAreaServer_PCBangPet','rsRecvAreaServer_PhenixPet','rsRecvAreaServer_HelpPet'])),
]
failed=[]
for n,ok in checks:
 print(('PASS' if ok else 'FAIL')+': '+n)
 if not ok: failed.append(n)
if failed: raise SystemExit(1)
print('pet runtime boundary: PASS')
