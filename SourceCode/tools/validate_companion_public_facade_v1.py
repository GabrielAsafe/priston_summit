#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
H=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.h').read_text(errors='replace')
Q=(ROOT/'Shared/gameplay/companions/v2/CompanionQuery.h').read_text(errors='replace')
CH=(ROOT/'SrcServer/src/systems/gameplay/companions/v2/legacy/CompanionLegacyCharacterHooks.h').read_text(errors='replace')
EX=(ROOT/'SrcServer/src/systems/gameplay/companions/v2/legacy/CompanionLegacyExperienceRuntime.h').read_text(errors='replace')
CHAR=(ROOT/'SrcServer/src/systems/gameplay/character/core/character.cpp').read_text(errors='replace')
PARTY=(ROOT/'SrcServer/src/core/server/legacy_runtime/PartySever.cpp').read_text(errors='replace')
ONS=(ROOT/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='replace')
PROF=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionProfileStore.cpp').read_text(errors='replace')
def fail(m): print(f'companion public facade v1: FAIL: {m}',file=sys.stderr); raise SystemExit(1)
def main():
 for bad in ('windows.h','rsPLAYINFO','smCHAR','TRANS_RECORD_DATA','INT64','DWORD','AwardExperience','ApplyAI','GetFollowPosition'):
  if bad in H: fail(f'public CompanionSystem facade leaks legacy runtime symbol: {bad}')
 for tok in ('CompanionCommands.h','CompanionLifecycle.h','CompanionQuery.h','CompanionUIIngress.h'):
  if tok not in H: fail(f'modern facade missing contract include: {tok}')
 if 'COMPANION_MAX_ACTIVE = 4u' not in Q: fail('max-active constant not moved to GREEN query contract')
 for tok in ('ApplyAI(smCHAR* character)','GetFollowPosition(smCHAR* character','ApplyIncomingDamage(smCHAR* target'):
  if tok not in CH: fail(f'private character hooks missing: {tok}')
 if 'AwardExperience(rsPLAYINFO* owner' not in EX: fail('private EXP runtime declaration missing')
 if 'CompanionLegacyCharacterHooks.h' not in CHAR or 'CompanionSystem.h' in CHAR: fail('character runtime still depends on public CompanionSystem facade')
 if 'CompanionLegacyExperienceRuntime.h' not in PARTY or 'CompanionSystem.h' in PARTY: fail('party runtime still depends on public CompanionSystem facade')
 if 'CompanionLegacyExperienceRuntime.h' not in ONS or 'systems/gameplay/companions/CompanionSystem.h' in ONS: fail('server runtime still depends on public CompanionSystem facade')
 if 'CompanionSystem.h' in PROF: fail('profile store still depends on legacy facade')
 for bad in ('COMPANION_MODE_AGGRESSIVE','COMPANION_MODE_PASSIVE'):
  if bad in PROF: fail(f'profile store still depends on private legacy mode: {bad}')
 print('companion public facade v1: PASS (public header is pointer-free; legacy character/EXP hooks are private)')
if __name__=='__main__': main()
