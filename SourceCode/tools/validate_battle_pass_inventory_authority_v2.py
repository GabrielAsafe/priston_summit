#!/usr/bin/env python3
from pathlib import Path
import re, sys
ROOT=Path(__file__).resolve().parents[1]
path=ROOT/'SrcServer/src/systems/progression/battle_pass/BattlePass.cpp'
text=path.read_text(encoding='utf-8', errors='replace')
checks=[
 ('inventory runtime included','systems/inventory/v2/legacy/LegacyInventoryRuntime.h' in text),
 ('inventory helper present','bool SummitBattlePassAddInventoryItem' in text),
 ('helper resolves PlayerId','ServerLegacyOnlinePlayerRegistry().IdOf(recipient)' in text),
 ('helper delegates exact add','ServerLegacyInventoryRuntime().Add(' in text and 'inventory::ItemInstanceId(' in text),
 ('helper requires applied result','InventoryMutationCode::Applied' in text),
 ('all reward sites use helper',text.count('SummitBattlePassAddInventoryItem(Player, TransItemInfo) && Player->lpsmSock') == 10),
 ('success packet gated by applied inventory mutation', text.count('if (SummitBattlePassAddInventoryItem(Player, TransItemInfo) && Player->lpsmSock)') == 10),
 ('direct legacy add retired','rsAddInvenItem(' not in text),
]
failed=[]
for name,ok in checks:
 print(('PASS' if ok else 'FAIL')+': '+name)
 if not ok: failed.append(name)
if failed:
 print('battle pass inventory authority v2: FAIL: '+', '.join(failed),file=sys.stderr); raise SystemExit(1)
print('battle pass inventory authority v2: PASS (10/10 reward item additions delegated)')
