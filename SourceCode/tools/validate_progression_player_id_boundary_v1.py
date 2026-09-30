#!/usr/bin/env python3
from pathlib import Path
import json, subprocess, sys
ROOT=Path(__file__).resolve().parents[1]
H=(ROOT/'SrcServer/src/systems/gameplay/progression/v2/ProgressionTriggerRuntime.h').read_text(errors='replace')
C=(ROOT/'SrcServer/src/systems/gameplay/progression/v2/ProgressionTriggerRuntime.cpp').read_text(errors='replace')
A=(ROOT/'tools/architecture/legacy_boundary_allowlist.txt').read_text(errors='replace')
def fail(m): print(f'progression player id boundary v1: FAIL: {m}',file=sys.stderr); raise SystemExit(1)
def main():
 for bad in ('rsPLAYINFO','class rsPLAYINFO','TryGetLegacy'):
  if bad in H or bad in C: fail(f'Progression trigger runtime retains pointer bridge: {bad}')
 for tok in ('ProcessServerProgressionOnlinePlayer(PlayerId player)','NotifyServerProgressionPlayerStateChanged(PlayerId player)'):
  if tok not in H: fail(f'PlayerId API missing: {tok}')
 for tok in ('ProcessServerProgressionOnlinePlayer(PlayerId playerId)','ServerLegacyPlayerAdapter().TryGet(playerId, snapshot)','NotifyServerProgressionPlayerStateChanged(PlayerId player)'):
  if tok not in C: fail(f'PlayerId runtime integration missing: {tok}')
 if 'ProgressionTriggerRuntime.h | symbol:rsPLAYINFO' in A or 'ProgressionTriggerRuntime.cpp | symbol:rsPLAYINFO' in A:
  fail('Progression pointer debt still allowlisted')
 print('progression player id boundary v1: PASS (Progression trigger runtime is pointer-free)')
if __name__=='__main__': main()
