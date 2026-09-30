#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
C=(ROOT/'Shared/gameplay/inventory/v2/Inventory.h').read_text(errors='replace')
H=(ROOT/'SrcServer/src/systems/inventory/v2/legacy/LegacyInventoryRuntime.h').read_text(errors='replace')
A=(ROOT/'SrcServer/src/systems/inventory/v2/legacy/LegacyInventoryRuntime.cpp').read_text(errors='replace')
def fail(m): print(f'inventory item grant v1: FAIL: {m}',file=sys.stderr); raise SystemExit(1)
def main():
 for tok in ('struct ItemGrant','enum class InventoryGrantCode','struct InventoryGrantResult','Grant(const ItemGrant& grant)','InvalidQuantity','LegacyRejected'):
  if tok not in C: fail(f'GREEN grant contract missing: {tok}')
 for bad in ('rsPutItemChecked','rsPLAYINFO','DWORD','DefaultItems','sinItem'):
  if bad in C: fail(f'GREEN grant contract leaks legacy symbol: {bad}')
 if 'InventoryGrantResult Grant(const ItemGrant& grant) override' not in H: fail('legacy runtime does not implement grant command')
 section=A.split('InventoryGrantResult LegacyInventoryRuntime::Grant',1)
 if len(section)!=2: fail('grant implementation missing')
 grant=section[1].split('LegacyInventoryRuntime& ServerLegacyInventoryRuntime',1)[0]
 for tok in ('grant.quantity != 1u','registry_.Resolve(grant.player)','rsPutItemChecked(','InventoryGrantCode::Granted','InventoryGrantCode::LegacyRejected'):
  if tok not in grant: fail(f'grant adapter token missing: {tok}')
 print('inventory item grant v1: PASS (default non-unique item grant isolated behind GREEN command)')
if __name__=='__main__': main()
