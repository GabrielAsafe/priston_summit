#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
C=(ROOT/'Shared/gameplay/companions/v2/CompanionCommands.h').read_text(errors='replace')
A=(ROOT/'SrcServer/src/systems/gameplay/companions/v2/legacy/LegacyCompanionCommandsAdapter.cpp').read_text(errors='replace')
O=(ROOT/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='replace')
def fail(m): print(f'companion commands boundary v1: FAIL: {m}',file=sys.stderr); raise SystemExit(1)
def main():
 for tok in ('class ICompanionCommands','Select(PlayerId owner','Summon(PlayerId owner','SummonSelected(PlayerId owner)','Dismiss(PlayerId owner','DismissAll(PlayerId owner)','SetMode(PlayerId owner, CompanionBehaviorMode mode)'):
  if tok not in C: fail(f'GREEN commands contract missing: {tok}')
 for bad in ('rsPLAYINFO','smCHAR','DWORD','windows.h','CompanionSystem.h'):
  if bad in C: fail(f'GREEN commands contract leaks legacy symbol: {bad}')
 for tok in ('players_.Resolve(owner)','companions::Select(','companions::Summon(','companions::Dismiss(','companions::DismissAll(','companions::SetMode('):
  if tok not in A: fail(f'legacy command adapter missing: {tok}')
 section=O[O.find('int len = rsCompString("/companion"'):O.find('int len = rsCompString("/companion"')+9000]
 for bad in ('gameplay::companions::Select(lpPlayInfo','gameplay::companions::Summon(lpPlayInfo','gameplay::companions::SummonSelected(lpPlayInfo','gameplay::companions::Dismiss(lpPlayInfo','gameplay::companions::DismissAll(lpPlayInfo','gameplay::companions::SetMode(lpPlayInfo'):
  if bad in section: fail(f'chat ingress still calls pointer command directly: {bad}')
 for tok in ('ServerLegacyCompanionCommandsAdapter()','companionCommands.Select(','companionCommands.Summon(','companionCommands.Dismiss(','companionCommands.SetMode('):
  if tok not in section: fail(f'chat ingress missing modern command consumer: {tok}')
 print('companion commands boundary v1: PASS (chat lifecycle commands route through PlayerId command port)')
if __name__=='__main__': main()
