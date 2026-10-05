#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[2]
checks={
 'SrcGame/src/systems/ui/v2/legacy/LegacyContextGateway.cpp':['!MeetsStatRequirements(cShop.ShowShopItem[index])'],
 'SrcGame/src/systems/commerce/legacy/sinShop.cpp':['pItem->sItemInfo.Level > sinChar->Level','MESSAGE_NO_USE_ITEM'],
 'Shared/server/legacy_runtime/OnSever_legacy_server.inl':['rsMeetsShopItemRequirements','item.Level <= character.Level','!rsMeetsShopItemRequirements(lpPlayInfo, lpTransBuyShopItem->sItem.sItemInfo)'],
}
errors=[]
for rel,tokens in checks.items():
 s=(root/rel).read_text(errors='replace')
 for t in tokens:
  if t not in s: errors.append(f'{rel}: missing {t}')
if errors:
 print('\n'.join('ERROR: '+x for x in errors));sys.exit(1)
print('PATCH 186 gate: OK')
