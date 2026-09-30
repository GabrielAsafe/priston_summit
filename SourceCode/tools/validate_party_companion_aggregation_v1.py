#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
G=(ROOT/'Shared/gameplay/companions/v2/PartyCompanionQuery.h').read_text(errors='replace')
P=(ROOT/'SrcServer/src/core/server/legacy_runtime/PartySever.cpp').read_text(errors='replace')
H=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.h').read_text(errors='replace')
C=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp').read_text(errors='replace')
def fail(m): print(f'party companion aggregation v1: FAIL: {m}',file=sys.stderr); raise SystemExit(1)
def main():
 for tok in ('ActivePartyCompanionsOnMap(','const social::party::IPartyQuery& partyQuery','const ICompanionQuery& companionQuery','companionQuery.ActiveCount(it->player)'):
  if tok not in G: fail(f'GREEN aggregation missing: {tok}')
 for bad in ('rsPLAYINFO','PARTY_PLAYER_MAX','lpPartyPlayers','DWORD'):
  if bad in G: fail(f'GREEN aggregation leaks legacy symbol: {bad}')
 for tok in ('ActivePartyCompanionsOnMap(','ServerLegacyPartyQueryAdapter()','ServerLegacyCompanionQueryAdapter()','ServerLegacyOnlinePlayerRegistry().IdOf(this)'):
  if tok not in P: fail(f'Party server modern integration missing: {tok}')
 if 'GetPartyActiveCount(rsPLAYINFO' in H or 'GetPartyActiveCount(rsPLAYINFO' in C:
  fail('legacy Companion GetPartyActiveCount API still exists')
 print('party companion aggregation v1: PASS (party+companion count composed in GREEN code; legacy Companion API removed)')
if __name__=='__main__': main()
