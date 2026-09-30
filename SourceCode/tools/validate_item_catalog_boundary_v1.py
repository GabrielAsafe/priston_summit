#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
C=(ROOT/'Shared/gameplay/inventory/v2/ItemCatalog.h').read_text(errors='replace')
A=(ROOT/'SrcServer/src/systems/gameplay/inventory/v2/legacy/LegacyItemCatalogAdapter.cpp').read_text(errors='replace')
def fail(m): print(f'item catalog boundary v1: FAIL: {m}',file=sys.stderr); raise SystemExit(1)
def main():
 for tok in ('enum class ItemCategory','struct ItemDefinition','class IItemCatalog','TryGetDefinition(ItemId item, ItemDefinition& out) const','TryGetDefinitionByContentKey(const std::string& contentKey, ItemDefinition& out) const','contentKey','displayName','stackable','defaultGrantable'):
  if tok not in C: fail(f'GREEN catalog token missing: {tok}')
 for bad in ('sITEM','sinItem','MAX_ITEM','ITEM_CLASS_','INVENTORY_POS_','DIRECTDRAW'):
  if bad in C: fail(f'GREEN catalog leaks legacy symbol: {bad}')
 for tok in ('#include "systems/inventory/legacy/sinItem.h"','for(int index = 0; index < MAX_ITEM; ++index)','legacyItem.CODE','legacyItem.LastCategory','ResolveDisplayName','ITEM_CLASS_POTION','ServerLegacyItemCatalogAdapter()'):
  if tok not in A: fail(f'legacy adapter token missing: {tok}')
 print('item catalog boundary v1: PASS (minimal canonical definition translated on demand from legacy sItem table)')
if __name__=='__main__': main()
