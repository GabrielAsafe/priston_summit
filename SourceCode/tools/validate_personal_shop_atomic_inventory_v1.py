#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT=Path(__file__).resolve().parents[1]
CONTRACT=(ROOT/'Shared/gameplay/commerce/v2/PersonalShopService.h').read_text()
ADAPTER=(ROOT/'SrcServer/src/systems/commerce/v2/legacy/LegacyPersonalShopAdapter.cpp').read_text()
HEADER=(ROOT/'SrcServer/src/systems/commerce/v2/legacy/LegacyPersonalShopAdapter.h').read_text()

def need(text, token, label):
    if token not in text:
        print(f'personal shop atomic inventory v1: FAIL: {label}: {token}', file=sys.stderr)
        raise SystemExit(1)

for token in ('BuyerInventoryRejected','RollbackFailed'):
    need(CONTRACT, token, 'result code')
for token in ('const inventory::IInventoryQuery& inventoryQuery','inventory::IInventoryCommands& inventoryCommands','inventoryQuery_','inventoryCommands_'):
    need(HEADER, token, 'injected inventory port')
for token in (
    'inventoryQuery_.Contains(request.seller, item).Present()',
    'inventoryCommands_.Add(request.buyer, item)',
    'inventoryCommands_.Remove(request.seller, item)',
    'const auto rollbackInventory',
    'const auto rollbackGold',
    'buyerAdd.code != inventory::InventoryMutationCode::Applied',
    'PersonalShopTradeCode::RollbackFailed',
    'ServerLegacyInventoryRuntime()',
):
    need(ADAPTER, token, 'atomic transfer')
for forbidden in ('rsFindInvenItem(', 'rsDeleteInvenItem(', 'rsAddInvenItem('):
    if forbidden in ADAPTER:
        print(f'personal shop atomic inventory v1: FAIL: direct inventory call remains: {forbidden}', file=sys.stderr)
        raise SystemExit(1)
print('personal shop atomic inventory v1: PASS (inventory port + buyer-capacity preflight + compensating rollback)')
