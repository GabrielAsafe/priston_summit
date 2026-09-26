#!/usr/bin/env python3
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[1]
protocol = (root/'Shared/gameplay/companions/CompanionProtocol.h').read_text(errors='ignore')
client = (root/'Shared/gameplay/character/character_legacy_client.inl').read_text(errors='ignore')
server_char = (root/'SrcServer/src/systems/gameplay/character/core/character.cpp').read_text(errors='ignore')
system = (root/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp').read_text(errors='ignore')
executor_h = (root/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.h').read_text(errors='ignore')
executor = (root/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp').read_text(errors='ignore')
bridge = (root/'SrcGame/src/systems/ui/v2/legacy/LegacyUIV2Bridge.cpp').read_text(errors='ignore')

checks = {
    'visual protocol v3+': ('COMPANION_VISUAL_PROTOCOL_VERSION = 3' in protocol or 'COMPANION_VISUAL_PROTOCOL_VERSION = 4' in protocol),
    'visual target coordinates in protocol': all(x in protocol for x in ('skillTargetX','skillTargetY','skillTargetZ')),
    'server runtime stores visual target coordinates': all(x in executor_h for x in ('lastSkillTargetX','lastSkillTargetY','lastSkillTargetZ')),
    'damage cast snapshots target position': 'target->pX, target->pY, target->pZ' in executor,
    'server replicates target position': all(x in system for x in (
        'entry.skillTargetX = slot.skillRuntime.lastSkillTargetX',
        'entry.skillTargetY = slot.skillRuntime.lastSkillTargetY',
        'entry.skillTargetZ = slot.skillRuntime.lastSkillTargetZ')),
    'client target proxy fallback': 'AcquireSkillTargetProxy' in client and 'g_skillTargetProxies[32]' in client and 'targetProxy=' in client,
    'visual crit state reset': 'character->AttackCritcal = 0;' in client,
    'hard teleport floor clearance reduced': 'safeTeleportY = y + (2 * fONE)' in server_char,
    'hard teleport client snap threshold': 'const int snap = 400 * fONE;' in client,
    'inventory world drop not globally blocked': 'ActiveTab() == CharacterTab::Inventory && MouseItem.Flag' not in bridge,
    'window bounds still block world click': 'characterWindow->GlobalBounds().Contains(Point{x, y})' in bridge,
}

failed = [name for name, ok in checks.items() if not ok]
if failed:
    print('Companion visual/drop/teleport V7: FAIL')
    for name in failed:
        print(' -', name)
    sys.exit(1)
print('Companion visual/drop/teleport V7: PASS')