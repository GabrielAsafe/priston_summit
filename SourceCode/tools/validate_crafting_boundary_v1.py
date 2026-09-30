#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
contract=(root/'Shared/gameplay/commerce/v2/CraftingService.h').read_text()
adapter=(root/'SrcServer/src/systems/commerce/v2/legacy/LegacyCraftingAdapter.cpp').read_text()
server=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text()
block=server.split('case OpCode::OPCODE_CRAFTITEM:',1)[1].split('case OpCode::OPCODE_AGINGITEM:',1)[0]
checks={
 'green service':'class ICraftingService' in contract and 'CraftResult Craft(const CraftRequest& request)' in contract,
 'green no legacy types':all(x not in contract for x in ('rsPLAYINFO','sCRAFTITEM_SERVER','sinTrade','SockInfo')),
 'ingress delegates':'ServerLegacyCraftingAdapter().Craft(request)' in block,
 'ingress no engine call':'rsRecvCraftItem(' not in block,
 'adapter owns engine':'rsRecvCraftItem(player, &packet)' in adapter,
 'adapter validates payload':'request.requestBytes.size() != sizeof(sCRAFTITEM_SERVER)' in adapter,
 'adapter resolves player':'registry_.Resolve(request.player)' in adapter,
}
for k,v in checks.items(): print(('PASS' if v else 'FAIL')+': '+k)
if not all(checks.values()): sys.exit(1)
print('crafting boundary v1: PASS (OPCODE_CRAFTITEM routed through ICraftingService; legacy formula engine preserved)')
