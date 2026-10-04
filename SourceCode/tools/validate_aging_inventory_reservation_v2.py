#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
service=(ROOT/'Shared/gameplay/commerce/v2/AgingService.h').read_text(errors='replace')
adapter=(ROOT/'SrcServer/src/systems/commerce/v2/legacy/LegacyAgingAdapter.cpp').read_text(errors='replace')
fail=[]
def need(text, needle, label):
    if needle not in text: fail.append(label)
need(service,'InventoryReservationRejected','AgingCode reservation rejection')
need(adapter,'AgingApplyReservations(packet)','Apply reservation set')
need(adapter,'packet.A_StoneItemCode','aging stone included')
need(adapter,'packet.Item.CODE, packet.Item.ItemHeader.Head, packet.Item.ItemHeader.dwChkSum','Upgrade item reservation')
need(adapter,'AgingCode::InventoryReservationRejected','conflict rejection')
for call in ('rsRecvAgingItem(player, &packet)','rsRecvAgingUpgradeItem(player, &packet)'):
    pos=adapter.find(call)
    if pos<0: fail.append('missing '+call); continue
    if 'LegacyCommerceReservationGuard reservation' not in adapter[max(0,pos-700):pos]:
        fail.append(call+' not reservation guarded')
if fail:
    [print('FAIL:',x,file=sys.stderr) for x in fail]; sys.exit(1)
print('aging inventory reservation v2: PASS')
