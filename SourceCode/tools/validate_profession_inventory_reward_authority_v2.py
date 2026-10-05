#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
text=(ROOT/'SrcServer/src/systems/crafting/professions/JOBS.cpp').read_text(encoding='utf-8',errors='replace')
checks=[
 ('inventory runtime included','systems/inventory/v2/legacy/LegacyInventoryRuntime.h' in text),
 ('reward helper present','bool SummitJobsAddInventoryItem' in text),
 ('reward helper resolves PlayerId','ServerLegacyOnlinePlayerRegistry().IdOf(recipient)' in text),
 ('reward helper delegates exact add','ServerLegacyInventoryRuntime().Add(' in text and 'inventory::ItemInstanceId(' in text),
 ('reward helper requires applied result','InventoryMutationCode::Applied' in text),
 ('three reward sites delegated',text.count('SummitJobsAddInventoryItem(Player, TransItemInfo) && Player->lpsmSock') == 3),
 ('direct legacy add retired','rsAddInvenItem(' not in text),
]
failed=[]
for name,ok in checks:
 print(('PASS' if ok else 'FAIL')+': '+name)
 if not ok: failed.append(name)
if failed:
 print('profession inventory reward authority v2: FAIL: '+', '.join(failed),file=sys.stderr);raise SystemExit(1)
print('profession inventory reward authority v2: PASS (3/3 legacy reward grants delegated)')
