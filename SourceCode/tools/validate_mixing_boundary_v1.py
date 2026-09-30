#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
contract=(root/'Shared/gameplay/commerce/v2/MixingService.h').read_text()
adapter=(root/'SrcServer/src/systems/commerce/v2/legacy/LegacyMixingAdapter.cpp').read_text()
server=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text()
block=server.split('case OpCode::OPCODE_MIXTURE_RESET_ITEM:',1)[1].split('case OpCode::OPCODE_SKIN_CHANGE_ITEM:',1)[0]
checks={
 'green service':'class IMixingService' in contract and 'MixingOperation' in contract,
 'green no legacy types':all(x not in contract for x in ('rsPLAYINFO','sMIXTURE_RESET_ITEM_SERVER','sinTrade')),
 'ingress delegates':'ServerLegacyMixingAdapter().Process(request)' in block,
 'ingress no engine':'rsRecvMixtureResetItem(' not in block,
 'adapter owns engine':'rsRecvMixtureResetItem(player, &packet)' in adapter,
 'adapter size guard':'request.requestBytes.size() != sizeof(sMIXTURE_RESET_ITEM_SERVER)' in adapter,
}
for k,v in checks.items(): print(('PASS' if v else 'FAIL')+': '+k)
if not all(checks.values()): sys.exit(1)
print('mixing boundary v1: PASS (mixture reset ingress isolated; legacy reset formula preserved)')
