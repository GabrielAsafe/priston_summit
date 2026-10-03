#!/usr/bin/env python3
from pathlib import Path
import re, sys
root=Path(__file__).resolve().parents[1]
cpp=(root/'SrcServer/src/systems/commerce/v2/legacy/LegacyTradeAdapter.cpp').read_text(errors='replace')
service=(root/'Shared/gameplay/commerce/v2/TradeTransactionService.h').read_text(errors='replace')
m=re.search(r'TradeExactCommitResult LegacyTradeAdapter::CommitExactTrade\([^)]*\)\s*\{(.*?)\n\}', cpp, re.S)
body=m.group(1) if m else ''
checks={
 'exact trade adapter delegates': 'transactionService_.CommitExactTrade(request)' in body,
 'adapter exact trade has no transfer orchestration': 'inventoryCommands_.Transfer' not in body,
 'adapter exact trade has no rollback lambda': 'rollback' not in body.lower(),
 'transaction kernel reserves inventory': 'inventoryReservations_.Reserve' in service,
 'transaction kernel rolls assets back': 'rollbackAssets' in service,
 'transaction kernel rolls gold back': 'rollbackGold' in service,
 'transaction kernel owns coin settlement': 'TransferCoins(' in service,
}
for n,ok in checks.items(): print(('PASS' if ok else 'FAIL')+': '+n)
if not all(checks.values()): sys.exit(1)
print(f'trade transaction boundary v2: PASS ({len(checks)}/{len(checks)})')
