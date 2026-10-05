#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
contract=(ROOT/'Shared/gameplay/commerce/v2/ShopService.h').read_text('utf-8',errors='ignore')
header=(ROOT/'Shared/gameplay/commerce/v2/ShopTransactionService.h').read_text('utf-8',errors='ignore')
adapter=(ROOT/'Shared/gameplay/commerce/v2/ShopTransactionService.h').read_text('utf-8',errors='ignore')
wiring=(ROOT/'SrcServer/src/systems/commerce/v2/legacy/LegacyShopAdapter.cpp').read_text('utf-8',errors='ignore')
runtime=(ROOT/'SrcServer/src/systems/inventory/v2/legacy/LegacyInventoryReservationRuntime.h').read_text('utf-8',errors='ignore')
checks=[]
def ck(ok,label): checks.append((ok,label)); print(('PASS' if ok else 'FAIL')+': '+label)
ck('InventoryReservationRejected' in contract,'NPC shop sale exposes reservation rejection')
ck('IInventoryReservationService& inventoryReservations' in header and 'inventoryReservations_' in header,'shop adapter consumes reservation boundary')
ck('ServerInventoryReservationService' in runtime,'shared server reservation runtime exists')
ck('ServerInventoryReservationService()' in wiring,'NPC shop uses shared reservation kernel')
start=adapter.find('ShopItemSaleResult SellItem')
end=adapter.find('ShopSaleResult CreditSale',start)
sell=adapter[start:end]
reserve=sell.find('inventoryReservations_.Reserve(reservationRequest)')
remove=sell.find('inventoryCommands_.Remove(request.player,request.item)')
credit=sell.find('walletCommands_.Credit(request.player,request.amount,WalletReason::NpcSellItem)')
ck(reserve>=0 and remove>=0 and reserve<remove,'NPC shop reserves item before removal')
ck(reserve>=0 and credit>=0 and reserve<credit,'NPC shop reserves item before wallet mutation')
ck('ReservationGuard reservationGuard' in sell and '~ReservationGuard()' in adapter and 'service_.Release(token_)' in adapter,'NPC shop releases reservation on every return path')
if not all(x for x,_ in checks):
    print('npc shop inventory reservation v1: FAIL',file=sys.stderr); sys.exit(1)
print(f'npc shop inventory reservation v1: PASS ({len(checks)}/{len(checks)})')
