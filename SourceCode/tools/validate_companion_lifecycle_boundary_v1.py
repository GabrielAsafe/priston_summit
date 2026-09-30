#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
C=(ROOT/'Shared/gameplay/companions/v2/CompanionLifecycle.h').read_text(errors='replace')
H=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.h').read_text(errors='replace')
L=(ROOT/'SrcServer/src/systems/gameplay/companions/v2/legacy/CompanionLegacyLifecycleRuntime.h').read_text(errors='replace')
A=(ROOT/'SrcServer/src/systems/gameplay/companions/v2/legacy/LegacyCompanionLifecycleAdapter.cpp').read_text(errors='replace')
O=(ROOT/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='replace')
def fail(m): print(f'companion lifecycle boundary v1: FAIL: {m}',file=sys.stderr); raise SystemExit(1)
def main():
 for tok in ('class ICompanionLifecycle','OwnerDisconnected(PlayerHandle owner)'):
  if tok not in C: fail(f'GREEN lifecycle contract missing: {tok}')
 for bad in ('rsPLAYINFO','smCHAR','DWORD','windows.h'):
  if bad in C: fail(f'GREEN lifecycle contract leaks legacy symbol: {bad}')
 if 'OnOwnerDisconnect(rsPLAYINFO' in H: fail('public CompanionSystem header retains pointer disconnect API')
 if 'OnOwnerDisconnect(rsPLAYINFO* owner)' not in L: fail('private legacy disconnect declaration missing')
 for tok in ('players_.Resolve(owner)','companions::OnOwnerDisconnect(legacyOwner)'):
  if tok not in A: fail(f'legacy lifecycle adapter missing: {tok}')
 section=O[O.find('int Serv_DisconnectPlayer'):O.find('int Serv_DisconnectPlayer')+1400]
 for tok in ('ServerLegacyOnlinePlayerRegistry().HandleOf(tpInfo)','ServerLegacyCompanionLifecycleAdapter().OwnerDisconnected(companionOwnerHandle)'):
  if tok not in section: fail(f'disconnect ingress missing modern lifecycle path: {tok}')
 if 'companions::OnOwnerDisconnect(tpInfo)' in section: fail('disconnect ingress still calls pointer runtime directly')
 print('companion lifecycle boundary v1: PASS (disconnect uses PlayerHandle lifecycle boundary)')
if __name__=='__main__': main()
