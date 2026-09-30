#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
contract=(root/'Shared/gameplay/commerce/v2/CraftingService.h').read_text()
adapter=(root/'SrcServer/src/systems/commerce/v2/legacy/LegacyCraftingAdapter.cpp').read_text()
server=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text()
ops={
 'MAKE_LINKCORE':('MakeLinkCore','rsRecvMakeLinkCore'),
 'USE_LINKCORE':('UseLinkCore','rsRecvUseLinkCore'),
 'WINGITEM':('Wing','rsRecvWingItem'),
 'SKIN_CHANGE_ITEM':('SkinChange','rsRecvSkinChangeItem'),
 'SMELTINGITEM':('Smelting','rsRecvSmeltingItem'),
 'MANUFACTURE':('Manufacture','rsRecvManufactureItem'),
}
checks={
 'green auxiliary contract':'enum class CraftingAuxiliaryOperation' in contract and 'ProcessAuxiliary' in contract,
 'green clean':all(x not in contract for x in ('rsPLAYINFO','TRANS_ITEMINFO','sCRAFTITEM_SERVER','sSKIN_CHANGE_SERVER','sSMELTINGITEM_SERVER','SManufactureItem_Server')),
}
for opcode,(operation,engine) in ops.items():
    start='case OpCode::OPCODE_'+opcode+':'
    block=server.split(start,1)[1].split('case OpCode::',1)[0]
    checks[opcode+' delegates']='CraftingAuxiliaryOperation::'+operation in block and 'ProcessAuxiliary(request)' in block
    checks[opcode+' no direct engine']=engine+'(' not in block
    checks[opcode+' adapter engine']=engine+'(player, &packet)' in adapter
for k,v in checks.items(): print(('PASS' if v else 'FAIL')+': '+k)
if not all(checks.values()): sys.exit(1)
print('crafting auxiliary boundary v1: PASS (link-core/wing/skin/smelting/manufacture ingress isolated)')
