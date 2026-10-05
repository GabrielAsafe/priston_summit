#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
H=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.h').read_text(errors='replace')
C=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp').read_text(errors='replace')
R=(ROOT/'SrcServer/src/systems/gameplay/companions/v2/CompanionRuntimeQuery.h').read_text(errors='replace')
A=(ROOT/'SrcServer/src/systems/gameplay/companions/v2/legacy/LegacyCompanionQueryAdapter.cpp').read_text(errors='replace')
O=(ROOT/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='replace')
I=(ROOT/'SrcServer/src/systems/network/v2/legacy/LegacyModernChatIngressAdapter.inl').read_text(errors='replace')
def fail(m): print(f'companion active count runtime v1: FAIL: {m}',file=sys.stderr); raise SystemExit(1)
def main():
 if 'GetActiveCount(rsPLAYINFO' in H or 'GetActiveCount(rsPLAYINFO' in C:
  fail('legacy GetActiveCount(rsPLAYINFO*) API still exists')
 for tok in ('std::uint32_t ActiveCount(PlayerId owner);',):
  if tok not in R: fail(f'modern runtime query missing: {tok}')
 for bad in ('rsPLAYINFO','smCHAR','windows.h','DWORD'):
  if bad in R: fail(f'modern runtime header leaks legacy symbol: {bad}')
 if 'std::uint32_t runtime::ActiveCount(PlayerId owner)' not in C:
  fail('Companion runtime active count function missing')
 if 'runtime::TryGetState(owner, state)' in C:
  for tok in ('OwnerState *FindState(PlayerId owner)','it->second.ownerHandle.id == owner','IsSlotActive(*state, state->slots[slot])'):
   if tok not in C: fail(f'Companion PlayerId state path missing: {tok}')
 else:
  for tok in ('state.ownerHandle.id != owner','IsSlotActive(state, state.slots[slot])'):
   if tok not in C: fail(f'Companion runtime active count missing: {tok}')
 if 'runtime::ActiveCount(owner)' not in A: fail('ICompanionQuery adapter does not consume PlayerId runtime query')
 for bad in ('players_.Resolve(owner)','companions::GetActiveCount('):
  if bad in A: fail(f'ICompanionQuery adapter still uses legacy pointer bridge: {bad}')
 migrated = sum(text.count('ServerLegacyCompanionQueryAdapter().ActiveCount(') + text.count('ServerLegacyCompanionQueryAdapter().TryGetState(') for text in (O,I))
 if migrated < 2:
  fail('legacy server consumers were not migrated to ICompanionQuery')
 print('companion active count runtime v1: PASS (active count is PlayerId-based; rsPLAYINFO API removed)')
if __name__=='__main__': main()
