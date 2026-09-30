#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
Q=(ROOT/'Shared/gameplay/companions/v2/CompanionQuery.h').read_text(errors='replace')
H=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.h').read_text(errors='replace')
C=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp').read_text(errors='replace')
A=(ROOT/'SrcServer/src/systems/gameplay/companions/v2/legacy/LegacyCompanionQueryAdapter.cpp').read_text(errors='replace')
O=(ROOT/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='replace')
def fail(m): print(f'companion state snapshot v1: FAIL: {m}',file=sys.stderr); raise SystemExit(1)
def main():
 for tok in ('struct CompanionStateSnapshot','std::uint32_t activeCount','CompanionBehaviorMode mode','std::string selectedName','TryGetState(PlayerId owner, CompanionStateSnapshot& out) const'):
  if tok not in Q: fail(f'GREEN state query missing: {tok}')
 for bad in ('rsPLAYINFO','smCHAR','DWORD','windows.h'):
  if bad in Q: fail(f'GREEN state snapshot leaks legacy symbol: {bad}')
 for text,name in ((H,'header'),(C,'implementation')):
  if 'GetMode(rsPLAYINFO' in text: fail(f'legacy GetMode pointer API remains in {name}')
  if 'GetSelectedName(rsPLAYINFO' in text: fail(f'legacy GetSelectedName pointer API remains in {name}')
 for tok in ('bool runtime::TryGetState(PlayerId owner, CompanionStateSnapshot& out)','out.selectedName = state->selectedName','out.mode = CompanionBehaviorMode::Follow','out.mode = CompanionBehaviorMode::Passive'):
  if tok not in C: fail(f'runtime snapshot integration missing: {tok}')
 if 'return runtime::TryGetState(owner, out);' not in A: fail('adapter does not expose runtime snapshot')
 for tok in ('CompanionStateSnapshot companionState','ServerLegacyCompanionQueryAdapter().TryGetState(','companionState.selectedName.empty()','static_cast<int>(companionState.mode)'):
  if tok not in O: fail(f'/companion status does not consume modern snapshot: {tok}')
 print('companion state snapshot v1: PASS (status state exposed by PlayerId; mode/selection pointer APIs removed)')
if __name__=='__main__': main()
