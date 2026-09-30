#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
C=(ROOT/'Shared/gameplay/social/party/v2/PartyQuery.h').read_text(errors='replace')
A=(ROOT/'SrcServer/src/systems/social/party/v2/legacy/LegacyPartyQueryAdapter.cpp').read_text(errors='replace')
P=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp').read_text(errors='replace')
def fail(m): print(f'companion party query boundary v1: FAIL: {m}',file=sys.stderr); raise SystemExit(1)
def main():
 for tok in ('struct PartyMemberSnapshot','struct PartySnapshot','class IPartyQuery','TryGetParty(PlayerId member, PartySnapshot& out) const'):
  if tok not in C: fail(f'GREEN party contract missing: {tok}')
 for bad in ('rsPLAYINFO','lpPartyPlayers','PARTY_PLAYER_MAX','PARTY_STATE_'):
  if bad in C: fail(f'GREEN party contract leaks legacy symbol: {bad}')
 for tok in ('legacyMember->lpPartyMaster','PARTY_STATE_MASTER','master->lpPartyPlayers[index]','players_.IdOf(legacyPartyMember)'):
  if tok not in A: fail(f'legacy party adapter token missing: {tok}')
 party_server=(ROOT/'SrcServer/src/core/server/legacy_runtime/PartySever.cpp').read_text(errors='replace')
 if 'GetPartyActiveCount(rsPLAYINFO' in P:
  section=P.split('int GetPartyActiveCount',1)[1].split('bool IsManagedCompanion',1)[0]
  for bad in ('lpPartyPlayers','PARTY_PLAYER_MAX'):
   if bad in section: fail(f'Companion party count retains direct party kernel read: {bad}')
 else:
  if 'ActivePartyCompanionsOnMap(' not in party_server: fail('party query no longer has a modern consumer')
 print('companion party query boundary v1: PASS (party membership/map reads isolated behind IPartyQuery)')
if __name__=='__main__': main()
