#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
CONTRACT = ROOT / 'Shared/gameplay/inventory/v2/Inventory.h'
ADAPTER = ROOT / 'SrcServer/src/systems/inventory/v2/legacy/LegacyInventoryRuntime.cpp'

def fail(message):
    print(f'inventory transfer boundary v1: FAIL: {message}', file=sys.stderr)
    raise SystemExit(1)

def main():
    contract = CONTRACT.read_text(errors='replace')
    adapter = ADAPTER.read_text(errors='replace')
    for token in ('enum class InventoryTransferCode', 'DestinationCapacityExceeded', 'RollbackFailed',
                  'InventoryTransferResult', 'Transfer(PlayerId source, PlayerId destination, ItemInstanceId item)'):
        if token not in contract:
            fail(f'contract token missing: {token}')
    for forbidden in ('rsPLAYINFO', 'rsAddInvenItem', 'rsDeleteInvenItem', 'rsFindInvenItem'):
        if forbidden in contract:
            fail(f'GREEN contract leaks legacy symbol: {forbidden}')
    for token in ('LegacyInventoryRuntime::Transfer(', 'Contains(source, item)', 'Contains(destination, item)',
                  'Add(destination, item)', 'Remove(source, item)', 'Remove(destination, item)',
                  'InventoryTransferCode::RollbackFailed'):
        if token not in adapter:
            fail(f'adapter token missing: {token}')
    add_pos = adapter.find('const InventoryMutationResult destinationAdd = Add(destination, item)')
    remove_pos = adapter.find('const InventoryMutationResult sourceRemove = Remove(source, item)', add_pos)
    rollback_pos = adapter.find('const InventoryMutationResult rollback = Remove(destination, item)', remove_pos)
    if not (0 <= add_pos < remove_pos < rollback_pos):
        fail('destination reservation/source remove/rollback ordering changed')
    print('inventory transfer boundary v1: PASS (capacity reservation + source remove + compensating rollback)')

if __name__ == '__main__':
    main()
