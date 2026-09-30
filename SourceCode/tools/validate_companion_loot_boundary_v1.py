#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
C=(ROOT/'Shared/gameplay/loot/v2/LootService.h').read_text(errors='replace')
A=(ROOT/'SrcServer/src/systems/gameplay/loot/v2/legacy/LegacyLootAdapter.cpp').read_text(errors='replace')
P=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp').read_text(errors='replace')
def fail(m): print(f'companion loot boundary v1: FAIL: {m}',file=sys.stderr); raise SystemExit(1)
def main():
 for tok in ('struct LootDropHandle','enum class LootPickupCode','class ILootService','Pickup(PlayerId player, LootDropHandle drop)'):
  if tok not in C: fail(f'GREEN loot contract missing: {tok}')
 for bad in ('STG_AREA','STG_ITEMS','rsPLAYINFO','DWORD','rsCompanionPickupStageItem'):
  if bad in C: fail(f'GREEN loot contract leaks legacy symbol: {bad}')
 for tok in ('LegacyLootAdapter::MakeHandle','area - StageArea','item - area->StgItems','LootPickupCode::StaleDrop','rsCompanionPickupStageItem('):
  if tok not in A: fail(f'legacy loot adapter token missing: {tok}')
 section=P.split('static bool TryHandleCompanionLoot',1)
 if len(section)!=2: fail('Companion loot handler missing')
 section=section[1].split('bool IsNameActive',1)[0]
 if 'rsCompanionPickupStageItem(' in section: fail('Companion still calls legacy pickup directly')
 for tok in ('LegacyLootAdapter::MakeHandle(area, item)','ServerLegacyLootAdapter().Pickup(ownerId, drop)','result.PickedUp()'):
  if tok not in section: fail(f'Companion loot integration missing: {tok}')
 print('companion loot boundary v1: PASS (ground pickup authority isolated behind ILootService with stale-drop handle validation)')
if __name__=='__main__': main()
