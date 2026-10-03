#!/usr/bin/env python3
from pathlib import Path
import re,sys
root=Path(__file__).resolve().parents[1]
cpp=(root/'SrcServer/src/systems/commerce/v2/legacy/LegacyPersonalShopAdapter.cpp').read_text(errors='replace')
kernel=(root/'Shared/gameplay/commerce/v2/PersonalShopTransactionService.h').read_text(errors='replace')
m=re.search(r'PersonalShopTradeResult LegacyPersonalShopAdapter::CommitTrade\([^)]*\)\s*\{(.*?)\n\}',cpp,re.S);body=m.group(1) if m else ''
checks={
 'adapter delegates transaction':'transactionService_.Commit(' in body,
 'legacy listing remains adapter':'sMyShopList' in body,
 'legacy logging remains adapter':'rsRecord_ItemLog_MyShop' in body,
 'adapter no wallet mutation':'walletCommands_' not in body,
 'adapter no inventory mutation':'inventoryCommands_' not in body,
 'adapter no rollback orchestration':'rollbackInventory' not in body and 'rollbackGold' not in body and 'rollbackCoin' not in body,
 'kernel owns reservation':'inventoryReservations_.Reserve' in kernel,
 'kernel owns three rollback domains':'rollbackInventory' in kernel and 'rollbackGold' in kernel and 'rollbackCoin' in kernel,
}
for n,ok in checks.items():print(('PASS' if ok else 'FAIL')+': '+n)
if not all(checks.values()):sys.exit(1)
print(f'personal shop transaction kernel v2: PASS ({len(checks)}/{len(checks)})')
