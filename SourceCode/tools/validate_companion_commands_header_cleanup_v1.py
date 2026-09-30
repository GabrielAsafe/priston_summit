#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
H=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.h').read_text(errors='replace')
L=(ROOT/'SrcServer/src/systems/gameplay/companions/v2/legacy/CompanionLegacyCommandsRuntime.h').read_text(errors='replace')
A=(ROOT/'SrcServer/src/systems/gameplay/companions/v2/legacy/LegacyCompanionCommandsAdapter.cpp').read_text(errors='replace')
O=(ROOT/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='replace')
def fail(m): print(f'companion commands header cleanup v1: FAIL: {m}',file=sys.stderr); raise SystemExit(1)
def main():
 for bad in ('int Summon(rsPLAYINFO','int SummonSelected(rsPLAYINFO','int Select(rsPLAYINFO','int Dismiss(rsPLAYINFO','int DismissAll(rsPLAYINFO','int SetMode(rsPLAYINFO','enum CompanionMode'):
  if bad in H: fail(f'public CompanionSystem header retains command surface: {bad}')
 for tok in ('enum CompanionMode','int Summon(rsPLAYINFO* owner','int Select(rsPLAYINFO* owner','int SetMode(rsPLAYINFO* owner, CompanionMode mode)'):
  if tok not in L: fail(f'private legacy command runtime missing: {tok}')
 if 'CompanionLegacyCommandsRuntime.h' not in A: fail('command adapter does not depend on private legacy runtime header')
 if 'CompanionMode mode =' in O or 'COMPANION_MODE_FOLLOW' in O or 'COMPANION_MODE_PASSIVE' in O:
  fail('chat ingress still parses legacy CompanionMode')
 print('companion commands header cleanup v1: PASS (pointer lifecycle commands moved out of public CompanionSystem header)')
if __name__=='__main__': main()
