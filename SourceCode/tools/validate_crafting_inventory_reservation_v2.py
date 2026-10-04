#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
service = (ROOT / 'Shared/gameplay/commerce/v2/CraftingService.h').read_text(errors='replace')
adapter = (ROOT / 'SrcServer/src/systems/commerce/v2/legacy/LegacyCraftingAdapter.cpp').read_text(errors='replace')
guard_path = ROOT / 'SrcServer/src/systems/commerce/v2/legacy/LegacyCommerceReservationGuard.h'
guard = guard_path.read_text(errors='replace') if guard_path.exists() else ''
fail = []

def need(text, needle, label):
    if needle not in text:
        fail.append(label)

need(service, 'InventoryReservationRejected', 'CraftingCode must expose reservation rejection')
need(guard, 'ServerInventoryReservationService()', 'commerce guard must use shared server reservation kernel')
need(guard, 'service_.Release(token_)', 'reservation lease must release by RAII')
need(adapter, 'CraftPacketReservations(packet)', 'Craft/Wing packets must enumerate reservations')
need(adapter, 'SkinChangeReservations(packet)', 'SkinChange must enumerate reservations')
need(adapter, 'SmeltingReservations(packet)', 'Smelting must enumerate reservations')
need(adapter, 'ManufactureReservations(packet)', 'Manufacture must enumerate reservations')
need(adapter, 'packet.Item.CODE, packet.Item.ItemHeader.Head, packet.Item.ItemHeader.dwChkSum', 'MakeLinkCore item must be reserved')
need(adapter, 'CraftingCode::InventoryReservationRejected', 'adapter must reject reservation conflicts')

# Every mutating legacy call must be preceded in its case by a reservation guard.
for call in ('rsRecvCraftItem(player, &packet)', 'rsRecvMakeLinkCore(player, &packet)',
             'rsRecvWingItem(player, &packet)', 'rsRecvSkinChangeItem(player, &packet)',
             'rsRecvSmeltingItem(player, &packet)', 'rsRecvManufactureItem(player, &packet)'):
    pos = adapter.find(call)
    if pos < 0:
        fail.append(f'missing legacy call {call}')
        continue
    window = adapter[max(0, pos - 900):pos]
    if 'LegacyCommerceReservationGuard reservation' not in window:
        fail.append(f'{call} is not guarded by a reservation lease')

# UseLinkCore deliberately only routes/validates; guard it against accidental inventory mutation.
kernel = (ROOT / 'SrcServer/src/systems/commerce/v2/legacy/LegacyCraftingKernel.inl').read_text(errors='replace')
start = kernel.find('int rsRecvUseLinkCore(')
end = kernel.find('\nint rsRecv', start + 1)
use_link = kernel[start:end if end >= 0 else len(kernel)]
if 'rsDeleteInvenItem' in use_link or 'rsAddInvenItem' in use_link:
    fail.append('UseLinkCore now mutates inventory; adapter reservation policy must be revisited')

if fail:
    for item in fail:
        print('FAIL:', item, file=sys.stderr)
    print(f'crafting inventory reservation v2: FAIL ({len(fail)} issues)', file=sys.stderr)
    sys.exit(1)
print('crafting inventory reservation v2: PASS')
