#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
C=(ROOT/'Shared/gameplay/inventory/v2/Inventory.h').read_text(errors='replace')
A=(ROOT/'SrcServer/src/systems/inventory/v2/legacy/LegacyInventoryRuntime.cpp').read_text(errors='replace')
def fail(m): print(f'inventory snapshot boundary v1: FAIL: {m}',file=sys.stderr); raise SystemExit(1)
def main():
 for tok in ('struct InventorySlot','struct InventorySnapshot','std::vector<InventorySlot> slots','TryGetSnapshot(PlayerId player, InventorySnapshot& out) const'):
  if tok not in C: fail(f'GREEN snapshot token missing: {tok}')
 for bad in ('rsPLAYINFO','InvenItemInfo','INVEN_ITEM_INFO_MAX','sTHROW_ITEM_INFO','WareHouseItemInfo'):
  if bad in C: fail(f'GREEN snapshot leaks legacy symbol: {bad}')
 for tok in ('!rsServerConfig.ServerInventoryMode','legacyPlayer->InvenItemInfo[index]','INVEN_ITEM_INFO_MAX','legacyItem.dwCode','legacyItem.dwKey','legacyItem.dwSum'):
  if tok not in A: fail(f'legacy snapshot token missing: {tok}')
 section=A.split('bool LegacyInventoryRuntime::TryGetSnapshot',1)[1].split('InventoryMutationResult LegacyInventoryRuntime::Add',1)[0]
 if 'WareHouseItemInfo' in section: fail('inventory snapshot must not merge warehouse state')
 print('inventory snapshot boundary v1: PASS (authoritative InvenItemInfo slots exposed without warehouse/UI leakage)')
if __name__=='__main__': main()
