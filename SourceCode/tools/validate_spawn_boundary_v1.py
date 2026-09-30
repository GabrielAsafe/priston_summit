#!/usr/bin/env python3
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[1]
contract = (root / 'Shared/gameplay/world/v2/ISpawnService.h').read_text(errors='replace')
adapter = (root / 'SrcServer/src/systems/gameplay/world/v2/legacy/LegacySpawnAdapter.cpp').read_text(errors='replace')
hells = (root / 'SrcServer/src/systems/gameplay/events/hells_gate/cHellsGate.cpp').read_text(errors='replace')
header = (root / 'SrcServer/src/systems/gameplay/events/hells_gate/cHellsGate.h').read_text(errors='replace')
checks = {
    'green spawn contract': all(x in contract for x in ('class ISpawnService', 'SpawnRequest', 'SpawnResult', 'virtual SpawnResult Spawn')),
    'legacy spawn delegated': 'OpenMonsterFromName(' in adapter and 'ServerLegacyOnlinePlayerRegistry()' in adapter,
    'spawn identity returned': 'spawned->dwObjectSerial' in adapter and 'EntityId(' in adapter,
    'hells gate uses spawn service': 'ServerLegacySpawnAdapter().Spawn' in hells and 'SpawnRequest(' in hells,
    'hells gate preserves template': 'Esp\\355rito do Mal' in hells,
    'hells gate preserves position': all(x in hells for x in ('MapId(23u)', 'WorldGridToFixed(-3624)', 'WorldGridToFixed(484)', 'WorldGridToFixed(-43172)')),
    'hells gate tracks entity id': 'gameplay::EntityId bossId' in header and 'bossId == monster' in hells and 'MonsterKilled(gameplay::EntityId monster)' in hells,
    'hells gate no direct spawn global': 'extern smCHAR *OpenMonsterFromName' not in hells and 'lpBoss = OpenMonsterFromName' not in hells,
}
failed = [k for k, v in checks.items() if not v]
if failed:
    print('spawn boundary v1: FAIL: ' + ', '.join(failed), file=sys.stderr)
    sys.exit(1)
print('spawn boundary v1: PASS')
