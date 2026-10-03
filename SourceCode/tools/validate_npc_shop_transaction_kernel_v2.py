#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
adapter=(root/'SrcServer/src/systems/commerce/v2/legacy/LegacyShopAdapter.cpp').read_text(errors='replace')
kernel=(root/'Shared/gameplay/commerce/v2/ShopTransactionService.h').read_text(errors='replace')
checks={
 'adapter delegates purchase':'transactionService_.Purchase(request)' in adapter,
 'adapter delegates exact purchase':'transactionService_.PurchaseItem(request)' in adapter,
 'adapter delegates potion purchase':'transactionService_.PurchasePotion(request)' in adapter,
 'adapter delegates exact sale':'transactionService_.SellItem(request)' in adapter,
 'adapter has no wallet mutation':'walletCommands_' not in adapter,
 'adapter has no inventory mutation':'inventoryCommands_' not in adapter,
 'kernel owns reservation':'inventoryReservations_.Reserve' in kernel,
 'kernel owns purchase rollback':'inventoryCommands_.Remove(request.player,request.item)' in kernel,
 'kernel owns sale rollback':'inventoryCommands_.Add(request.player,request.item)' in kernel,
}
for n,ok in checks.items():print(('PASS' if ok else 'FAIL')+': '+n)
if not all(checks.values()):sys.exit(1)
print(f'npc shop transaction kernel v2: PASS ({len(checks)}/{len(checks)})')
