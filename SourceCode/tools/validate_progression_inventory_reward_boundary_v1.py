#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
H=(ROOT/'SrcServer/src/systems/gameplay/progression/v2/LegacyProgressionGateways.h').read_text(errors='replace')
C=(ROOT/'SrcServer/src/systems/gameplay/progression/v2/LegacyProgressionGateways.cpp').read_text(errors='replace')
R=(ROOT/'SrcServer/src/systems/gameplay/progression/v2/ProgressionRuntime.cpp').read_text(errors='replace')
def fail(m): print(f'progression inventory reward boundary v1: FAIL: {m}',file=sys.stderr); raise SystemExit(1)
def main():
 for tok in ('const inventory::IItemCatalog& itemCatalog','inventory::IInventoryCommands& inventoryCommands','ResolveGrantableItem'):
  if tok not in H: fail(f'gateway contract missing: {tok}')
 for bad in ('#include "systems/inventory/legacy/sinItem.h"','DefaultItems','DefaultItemCount','sItem[','rsPutItemChecked'):
  if bad in C: fail(f'progression reward gateway retains direct item legacy authority: {bad}')
 for tok in ('itemCatalog_.TryGetDefinitionByContentKey','out.defaultGrantable','inventoryCommands_.Grant(','inventory::ItemGrant(playerId, definition.id, 1u)'):
  if tok not in C: fail(f'progression item reward integration missing: {tok}')
 for tok in ('ServerLegacyItemCatalogAdapter()','ServerLegacyInventoryRuntime()'):
  if tok not in R: fail(f'runtime wiring missing: {tok}')
 print('progression inventory reward boundary v1: PASS (item reward lookup/grant routes through Inventory ports)')
if __name__=='__main__': main()
