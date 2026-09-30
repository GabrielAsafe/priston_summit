#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
H=(ROOT/'SrcServer/src/systems/commerce/v2/legacy/LegacyPersonalShopAdapter.h').read_text(errors='replace')
A=(ROOT/'SrcServer/src/systems/commerce/v2/legacy/LegacyPersonalShopAdapter.cpp').read_text(errors='replace')
def fail(m): print(f'personal shop potion stack v1: FAIL: {m}',file=sys.stderr); raise SystemExit(1)
def main():
 for tok in ('const inventory::IPotionStackQuery& potionStackQuery','inventory::IPotionStackCommands& potionStackCommands','potionStackQuery_','potionStackCommands_'):
  if tok not in H: fail(f'injected stack port missing: {tok}')
 for tok in ('potionStackQuery_.TryGetStack(','potionStackCommands_.TransferStack(','request.seller, request.buyer, ItemId(request.itemCode)','request.buyer, request.seller, ItemId(request.itemCode)','PotionStackTransferCode::RollbackFailed','ServerLegacyPotionStackAdapter()','rsServerPotionErrorLog'):
  if tok not in A: fail(f'stack path token missing: {tok}')
 for bad in ('rsGetServerPotion(', 'rsAddServerPotion('):
  if bad in A: fail(f'direct potion stack authority remains: {bad}')
 print('personal shop potion stack v1: PASS (availability + transfer + reverse rollback behind potion stack port)')
if __name__=='__main__': main()
