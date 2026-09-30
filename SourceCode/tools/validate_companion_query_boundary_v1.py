#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
C=(ROOT/'Shared/gameplay/companions/v2/CompanionQuery.h').read_text(errors='replace')
A=(ROOT/'SrcServer/src/systems/gameplay/companions/v2/legacy/LegacyCompanionQueryAdapter.cpp').read_text(errors='replace')
P=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp').read_text(errors='replace')
def fail(m): print(f'companion query boundary v1: FAIL: {m}',file=sys.stderr); raise SystemExit(1)
def main():
 for tok in ('class ICompanionQuery','ActiveCount(PlayerId owner) const'):
  if tok not in C: fail(f'GREEN companion query missing: {tok}')
 for bad in ('rsPLAYINFO','smCHAR','DWORD','CompanionSystem.h'):
  if bad in C: fail(f'GREEN companion query leaks legacy symbol: {bad}')
 if 'runtime::ActiveCount(owner)' in A:
  if 'players_.Resolve(owner)' in A or 'companions::GetActiveCount(' in A:
   fail('modern companion adapter still resolves rsPLAYINFO/GetActiveCount')
 elif 'players_.Resolve(owner)' not in A or 'companions::GetActiveCount(legacyOwner)' not in A:
  fail('legacy companion query adapter has neither modern nor historical active-count path')
 if 'ServerLegacyCompanionQueryAdapter' not in A: fail('server companion query adapter factory missing')
 party_server=(ROOT/'SrcServer/src/core/server/legacy_runtime/PartySever.cpp').read_text(errors='replace')
 if 'GetPartyActiveCount(rsPLAYINFO' in P:
  section=P.split('int GetPartyActiveCount',1)[1].split('bool IsManagedCompanion',1)[0]
  if 'ServerLegacyOnlinePlayerRegistry().Resolve(it->player)' in section: fail('party active count still resolves member pointer directly')
  if 'ServerLegacyCompanionQueryAdapter().ActiveCount(it->player)' not in section: fail('party active count does not consume ICompanionQuery adapter')
 else:
  if 'ServerLegacyCompanionQueryAdapter()' not in party_server or 'ActivePartyCompanionsOnMap(' not in party_server:
   fail('Party server does not consume ICompanionQuery through GREEN aggregation')
 print('companion query boundary v1: PASS (active companion count exposed by PlayerId and consumed by party integration)')
if __name__=='__main__': main()
