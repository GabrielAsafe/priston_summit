#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
G=(ROOT/'Shared/gameplay/companions/v2/CompanionUIIngress.h').read_text(errors='replace')
H=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.h').read_text(errors='replace')
L=(ROOT/'SrcServer/src/systems/gameplay/companions/v2/legacy/CompanionLegacyUIRuntime.h').read_text(errors='replace')
A=(ROOT/'SrcServer/src/systems/gameplay/companions/v2/legacy/LegacyCompanionUIIngressAdapter.cpp').read_text(errors='replace')
O=(ROOT/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='replace')
R=(ROOT/'SrcServer/src/systems/network/v2/legacy/LegacyGameplayPacketRouter.cpp').read_text(errors='replace')
def fail(m): print(f'companion ui ingress boundary v1: FAIL: {m}',file=sys.stderr); raise SystemExit(1)
def main():
 for tok in ('class ICompanionUIIngress','HandleAction(PlayerId owner, const protocol::CompanionUIActionPacket& packet)'):
  if tok not in G: fail(f'GREEN UI ingress missing: {tok}')
 for bad in ('rsPLAYINFO','smCHAR','DWORD','windows.h'):
  if bad in G: fail(f'GREEN UI ingress leaks legacy symbol: {bad}')
 for bad in ('SendUIState(rsPLAYINFO','HandleUIAction(rsPLAYINFO','CompanionProtocol.h'):
  if bad in H: fail(f'public CompanionSystem header retains UI runtime surface: {bad}')
 for tok in ('SendUIState(rsPLAYINFO* owner','HandleUIAction(rsPLAYINFO* owner'):
  if tok not in L: fail(f'private UI runtime missing: {tok}')
 for tok in ('players_.Resolve(owner)','companions::HandleUIAction(legacyOwner, &packet)'):
  if tok not in A: fail(f'UI adapter missing legacy bridge: {tok}')
 start=O.find('case gameplay::companions::protocol::OPCODE_COMPANION_UI_ACTION')
 section=O[start:start+900] if start >= 0 else ''
 routed=(('RouteServerGameplayPacket(' in section or 'RouteServerGameplayPacket(' in O) and 'ServerLegacyCompanionUIIngressAdapter().HandleAction(' in R)
 direct='ServerLegacyCompanionUIIngressAdapter().HandleAction(' in section
 if not (routed or direct): fail('packet ingress does not use ICompanionUIIngress adapter')
 if 'companions::HandleUIAction(lpPlayInfo' in section: fail('packet ingress still calls pointer UI runtime')
 print('companion ui ingress boundary v1: PASS (UI packet ingress routes PlayerId through GREEN port)')
if __name__=='__main__': main()
