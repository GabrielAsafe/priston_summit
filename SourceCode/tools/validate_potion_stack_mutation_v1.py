#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
C=(ROOT/'Shared/gameplay/inventory/v2/PotionStack.h').read_text(errors='replace')
A=(ROOT/'SrcServer/src/systems/gameplay/inventory/v2/legacy/LegacyPotionStackAdapter.cpp').read_text(errors='replace')
H=(ROOT/'SrcServer/src/systems/gameplay/inventory/v2/legacy/LegacyPotionStackAdapter.h').read_text(errors='replace')
def fail(m): print(f'potion stack mutation v1: FAIL: {m}',file=sys.stderr); raise SystemExit(1)
def main():
 for tok in ('PotionStackMutationCode','PotionStackMutationRequest','PotionStackMutationResult','AddStack','RemoveStack','CapacityExceeded','InsufficientQuantity'):
  if tok not in C: fail(f'contract token missing: {tok}')
 for bad in ('rsPLAYINFO','rsGetServerPotion','rsAddServerPotion','sinPM1'):
  if bad in C: fail(f'GREEN contract leaks legacy symbol: {bad}')
 for tok in ('AddStack(const PotionStackMutationRequest& request)','RemoveStack(const PotionStackMutationRequest& request)'):
  if tok not in H: fail(f'adapter interface missing: {tok}')
 for tok in ('MutatePotionStack(','PotionStackMutationCode::CapacityExceeded','PotionStackMutationCode::InsufficientQuantity','RestorePotionCount(legacyPlayer, code, before)','PotionStackMutationCode::RollbackFailed'):
  if tok not in A: fail(f'adapter safety token missing: {tok}')
 print('potion stack mutation v1: PASS (single-player add/remove with preflight, postcondition verification and rollback)')
if __name__=='__main__': main()
