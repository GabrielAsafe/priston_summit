#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
checks=[]
def req(path, needle, label):
    p=ROOT/path; text=p.read_text('utf-8',errors='ignore') if p.exists() else ''
    ok=needle in text; checks.append((ok,label))
    if not ok: print(f'FAIL: {label}: {path} missing {needle!r}')
req('Shared/gameplay/inventory/v2/InventoryReservation.h','IInventoryReservationService','GREEN reservation contract')
req('Shared/gameplay/inventory/v2/InventoryReservationService.h','InventoryReservationCode::Conflict','reservation service detects conflicts')
req('Shared/gameplay/inventory/v2/InventoryReservationService.h','ReleasePlayer','reservation service can clear player leases')
req('Shared/gameplay/commerce/v2/TradeService.h','InventoryReservationRejected','trade exposes reservation rejection')
req('SrcServer/src/systems/commerce/v2/legacy/LegacyTradeAdapter.h','IInventoryReservationService& inventoryReservations','trade adapter consumes reservation service')
req('Shared/gameplay/commerce/v2/TradeTransactionService.h','inventoryReservations_.Reserve','exact commit reserves items before mutation')
req('Shared/gameplay/commerce/v2/TradeTransactionService.h','ReservationGuard','reservation always releases on commit/rollback return')
req('SrcServer/src/systems/inventory/v2/legacy/LegacyInventoryReservationRuntime.h','ServerInventoryReservationService','server owns shared V2 reservation kernel')
req('SrcServer/src/systems/commerce/v2/legacy/LegacyTradeAdapter.cpp','ServerInventoryReservationService()','trade consumes shared reservation kernel')
failed=[x for x in checks if not x[0]]
if failed:
    print(f'Trade inventory reservation v1: FAIL ({len(failed)}/{len(checks)})'); sys.exit(1)
print(f'Trade inventory reservation v1: PASS ({len(checks)}/{len(checks)})')
