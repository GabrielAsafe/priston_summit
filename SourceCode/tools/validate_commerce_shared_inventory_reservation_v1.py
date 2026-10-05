#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]

def read(path):
    return (ROOT / path).read_text('utf-8', errors='ignore')

runtime = read('SrcServer/src/systems/inventory/v2/legacy/LegacyInventoryReservationRuntime.h')
trade = read('SrcServer/src/systems/commerce/v2/legacy/LegacyTradeAdapter.cpp')
shop_h = read('Shared/gameplay/commerce/v2/PersonalShopTransactionService.h')
shop = read('Shared/gameplay/commerce/v2/PersonalShopTransactionService.h')
shop_wiring = read('SrcServer/src/systems/commerce/v2/legacy/LegacyPersonalShopAdapter.cpp')
contract = read('Shared/gameplay/commerce/v2/PersonalShopService.h')

checks = []
def check(ok, label):
    checks.append((bool(ok), label))
    print(('PASS' if ok else 'FAIL') + ': ' + label)

check('ServerInventoryReservationService' in runtime and 'static InventoryReservationService service;' in runtime,
      'one server-wide inventory reservation kernel is defined')
check('InventoryReservationRejected' in contract,
      'personal shop exposes reservation conflict result')
check('IInventoryReservationService& inventoryReservations' in shop_h and 'inventoryReservations_' in shop_h,
      'personal shop consumes reservation boundary')
check('inventoryReservations_.Reserve(reservationRequest)' in shop,
      'personal shop reserves seller item')
check('ReservationGuard' in shop and 'service_.Release(token_)' in shop,
      'personal shop reservation is released on every return path')
check('ServerInventoryReservationService()' in trade,
      'trade uses shared reservation kernel')
check('ServerInventoryReservationService()' in shop_wiring,
      'personal shop uses shared reservation kernel')
check('static inventory::InventoryReservationService inventoryReservations' not in trade,
      'trade no longer owns an isolated reservation service')

reserve_pos = shop.find('inventoryReservations_.Reserve(reservationRequest)')
first_mutations = [
    shop.find('inventoryCommands_.Add(request.buyer, item)', reserve_pos + 1),
    shop.find('inventoryCommands_.Remove(request.seller, item)', reserve_pos + 1),
    shop.find('potionStackCommands_.TransferStack', reserve_pos + 1),
    shop.find('walletCommands_.Debit(request.buyer', reserve_pos + 1),
    shop.find('coinCommands_.DebitCoins(request.buyer', reserve_pos + 1),
]
first_mutations = [p for p in first_mutations if p >= 0]
check(reserve_pos >= 0 and first_mutations and reserve_pos < min(first_mutations),
      'reservation occurs before inventory/wallet/coin mutation')

failed = [label for ok, label in checks if not ok]
if failed:
    print(f'commerce shared inventory reservation v1: FAIL ({len(failed)}/{len(checks)})', file=sys.stderr)
    sys.exit(1)
print(f'commerce shared inventory reservation v1: PASS ({len(checks)}/{len(checks)})')
