#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
H=(ROOT/'SrcServer/src/systems/gameplay/progression/v2/LegacyProgressionGateways.h').read_text(errors='replace')
C=(ROOT/'SrcServer/src/systems/gameplay/progression/v2/LegacyProgressionGateways.cpp').read_text(errors='replace')
R=(ROOT/'SrcServer/src/systems/gameplay/progression/v2/ProgressionRuntime.cpp').read_text(errors='replace')
def fail(m): print(f'progression player registry boundary v1: FAIL: {m}',file=sys.stderr); raise SystemExit(1)
def main():
 for tok in ('const player::legacy::LegacyOnlinePlayerRegistry& players','players_'):
  if tok not in H: fail(f'gateway registry dependency missing: {tok}')
 for bad in ('for (int i = 0; i < CONNECTMAX; ++i)','rsPlayInfo[i]','FindOnlinePlayer(','EqualsIgnoreCase('):
  if bad in C: fail(f'progression gateway retains manual online-player scan: {bad}')
 if C.count('players_.ResolveByPlayerKey(playerKey)') < 2: fail('reward/story gateways do not both use player registry')
 if R.count('player::legacy::ServerLegacyOnlinePlayerRegistry()') < 2: fail('runtime registry wiring missing')
 print('progression player registry boundary v1: PASS (reward/story player lookup uses authoritative online registry)')
if __name__=='__main__': main()
