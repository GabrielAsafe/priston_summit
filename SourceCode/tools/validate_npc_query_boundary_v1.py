#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
contract=(root/'Shared/gameplay/world/v2/INpcQuery.h').read_text(errors='replace')
adapter=(root/'SrcServer/src/systems/gameplay/world/v2/legacy/LegacyNpcAdapter.cpp').read_text(errors='replace')
combat=(root/'Shared/gameplay/combat/Svr_Damge_legacy_server.inl').read_text(errors='replace')
interaction=(root/'SrcServer/src/systems/gameplay/world/v2/legacy/LegacyInteractionAdapter.cpp').read_text(errors='replace') if (root/'SrcServer/src/systems/gameplay/world/v2/legacy/LegacyInteractionAdapter.cpp').exists() else ''
checks={
 'green npc contract': all(x in contract for x in ('struct NpcSnapshot','class INpcQuery','TryGetNpc(EntityId')),
 'legacy resolves entity': 'srFindCharFromSerial' in adapter,
 'legacy preserves event code': 'character->smMonsterInfo.EventCode' in adapter,
 'hells gate consumes npc query': (('ServerLegacyNpcAdapter().TryGetNpc' in combat and 'npcSnapshot.eventCode == 988u' in combat) or ('ServerLegacyInteractionAdapter().Interact' in combat and 'ServerLegacyNpcAdapter().TryGetNpc' in interaction and 'npc.eventCode == 988u' in interaction)),
 'hells gate no direct 988 event field': 'lpChar->smMonsterInfo.EventCode == 988' not in combat,
}
failed=[k for k,v in checks.items() if not v]
if failed:
 print('npc query boundary v1: FAIL: '+', '.join(failed), file=sys.stderr); sys.exit(1)
print('npc query boundary v1: PASS')
