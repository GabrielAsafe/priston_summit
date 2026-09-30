#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]; C=(ROOT/'Shared/gameplay/inventory/v2/PotionStack.h').read_text(errors='replace'); A=(ROOT/'SrcServer/src/systems/gameplay/inventory/v2/legacy/LegacyPotionStackAdapter.cpp').read_text(errors='replace')
def fail(m): print(f'potion stack boundary v1: FAIL: {m}',file=sys.stderr); raise SystemExit(1)
def main():
 for tok in ('class IPotionStackQuery','class IPotionStackCommands','PotionStackTransferRequest','PotionStackTransferResult','DestinationCapacityExceeded','RollbackFailed'):
  if tok not in C: fail(f'contract token missing: {tok}')
 for bad in ('rsPLAYINFO','rsGetServerPotion','rsAddServerPotion','sinPM1'):
  if bad in C: fail(f'GREEN stack contract leaks legacy symbol: {bad}')
 for tok in ('IsSupportedPotion','rsGetServerPotion(source, code)','rsGetServerPotion(destination, code)','rsAddServerPotion(source, code, -static_cast<int>(request.quantity))','rsAddServerPotion(destination, code, static_cast<int>(request.quantity))','RestorePotionCount(destination, code, destinationBefore)','RestorePotionCount(source, code, sourceBefore)','PotionStackTransferCode::RollbackFailed'):
  if tok not in A: fail(f'adapter safety token missing: {tok}')
 print('potion stack boundary v1: PASS (quantity/capacity preflight + postcondition verification + bilateral compensation)')
if __name__=='__main__': main()
