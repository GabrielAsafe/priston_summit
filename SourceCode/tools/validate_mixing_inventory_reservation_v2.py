#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
service=(ROOT/'Shared/gameplay/commerce/v2/MixingService.h').read_text(errors='replace')
adapter=(ROOT/'SrcServer/src/systems/commerce/v2/legacy/LegacyMixingAdapter.cpp').read_text(errors='replace')
fail=[]
def need(text, needle, label):
    if needle not in text: fail.append(label)
need(service,'InventoryReservationRejected','MixingCode reservation rejection')
need(adapter,'packet.DesMixtureResetItem.sItemInfo.CODE','target item reservation')
need(adapter,'packet.dwMixtureResetStoneItemCode','reset stone reservation')
need(adapter,'LegacyCommerceReservationGuard reservation','shared reservation guard')
need(adapter,'MixingCode::InventoryReservationRejected','conflict rejection')
pos=adapter.find('rsRecvMixtureResetItem(player, &packet)')
if pos < 0 or 'LegacyCommerceReservationGuard reservation' not in adapter[max(0,pos-800):pos]:
    fail.append('legacy reset kernel is not reservation guarded')
if fail:
    [print('FAIL:',x,file=sys.stderr) for x in fail]; sys.exit(1)
print('mixing inventory reservation v2: PASS')
