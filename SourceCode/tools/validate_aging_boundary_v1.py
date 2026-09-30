#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
contract=(root/'Shared/gameplay/commerce/v2/AgingService.h').read_text()
adapter=(root/'SrcServer/src/systems/commerce/v2/legacy/LegacyAgingAdapter.cpp').read_text()
server=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text()
apply=server.split('case OpCode::OPCODE_AGINGITEM:',1)[1].split('case OpCode::OPCODE_AGING_UPGRADE:',1)[0]
upgrade=server.split('case OpCode::OPCODE_AGING_UPGRADE:',1)[1].split('case OpCode::OPCODE_MAKE_LINKCORE:',1)[0]
checks={
 'green service':'class IAgingService' in contract and 'AgingOperation' in contract,
 'green no legacy types':all(x not in contract for x in ('rsPLAYINFO','sCRAFTITEM_SERVER','TRANS_ITEMINFO','sinTrade')),
 'apply delegates':'ServerLegacyAgingAdapter().Process(request)' in apply and 'AgingOperation::Apply' in apply,
 'upgrade delegates':'ServerLegacyAgingAdapter().Process(request)' in upgrade and 'AgingOperation::Upgrade' in upgrade,
 'handlers no engines':'rsRecvAgingItem(' not in apply and 'rsRecvAgingUpgradeItem(' not in upgrade,
 'adapter owns apply':'rsRecvAgingItem(player, &packet)' in adapter,
 'adapter owns upgrade':'rsRecvAgingUpgradeItem(player, &packet)' in adapter,
 'size guards':'sizeof(sCRAFTITEM_SERVER)' in adapter and 'sizeof(TRANS_ITEMINFO)' in adapter,
}
for k,v in checks.items(): print(('PASS' if v else 'FAIL')+': '+k)
if not all(checks.values()): sys.exit(1)
print('aging boundary v1: PASS (aging apply/upgrade ingress isolated; legacy aging engine preserved)')
