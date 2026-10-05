#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
Q=(ROOT/'Shared/gameplay/companions/v2/CompanionQuery.h').read_text(errors='replace')
C=(ROOT/'Shared/gameplay/companions/v2/CompanionCommands.h').read_text(errors='replace')
H=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.h').read_text(errors='replace')
R=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp').read_text(errors='replace')
QA=(ROOT/'SrcServer/src/systems/gameplay/companions/v2/legacy/LegacyCompanionQueryAdapter.cpp').read_text(errors='replace')
CA=(ROOT/'SrcServer/src/systems/gameplay/companions/v2/legacy/LegacyCompanionCommandsAdapter.cpp').read_text(errors='replace')
O=(ROOT/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='replace')
I=(ROOT/'SrcServer/src/systems/network/v2/legacy/LegacyModernChatIngressAdapter.inl').read_text(errors='replace')
def fail(m): print(f'companion crystal boundary v1: FAIL: {m}',file=sys.stderr); raise SystemExit(1)
def main():
 if 'ConfiguredCrystalItemCode() const' not in Q: fail('crystal config missing from GREEN query contract')
 if 'UseCrystal(PlayerId owner, std::uint32_t itemCode)' not in C: fail('crystal use missing from GREEN command contract')
 for text,name in ((Q,'query'),(C,'commands')):
  for bad in ('rsPLAYINFO','DWORD','windows.h','CompanionSystem.h'):
   if bad in text: fail(f'GREEN {name} contract leaks legacy symbol: {bad}')
 for bad in ('GetConfiguredCrystalItemCode','HandleCrystalUse(rsPLAYINFO'):
  if bad in H: fail(f'public CompanionSystem crystal API remains: {bad}')
 for tok in ('std::uint32_t runtime::ConfiguredCrystalItemCode()','const std::uint32_t configuredCode = runtime::ConfiguredCrystalItemCode()'):
  if tok not in R: fail(f'crystal runtime integration missing: {tok}')
 if 'return runtime::ConfiguredCrystalItemCode();' not in QA: fail('query adapter does not expose crystal config')
 if 'companions::HandleCrystalUse(legacyOwner, itemCode)' not in CA: fail('command adapter does not bridge crystal use')
 if O.count('ServerLegacyCompanionQueryAdapter().ConfiguredCrystalItemCode()') + I.count('ServerLegacyCompanionQueryAdapter().ConfiguredCrystalItemCode()') < 3: fail('server crystal config consumers not migrated')
 if 'ServerLegacyCompanionCommandsAdapter().UseCrystal(' not in O: fail('server item-use ingress not migrated')
 print('companion crystal boundary v1: PASS (crystal config/query and use command are pointer-free at server ingress)')
if __name__=='__main__': main()
