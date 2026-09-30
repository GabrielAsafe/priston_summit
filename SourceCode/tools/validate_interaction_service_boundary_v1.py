#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
contract=(root/'Shared/gameplay/world/v2/IInteractionService.h').read_text(errors='replace')
adapter=(root/'SrcServer/src/systems/gameplay/world/v2/legacy/LegacyInteractionAdapter.cpp').read_text(errors='replace')
combat=(root/'Shared/gameplay/combat/Svr_Damge_legacy_server.inl').read_text(errors='replace')
checks={
 'green interaction contract': all(x in contract for x in ('struct InteractionRequest','struct InteractionResult','class IInteractionService','Interact(const InteractionRequest&')),
 'adapter resolves npc query': 'ServerLegacyNpcAdapter().TryGetNpc' in adapter,
 'adapter owns hells gate route': 'npc.eventCode == 988u' in adapter and 'cHg.SendOpenNpc(request.player)' in adapter,
 'combat consumes interaction service': 'ServerLegacyInteractionAdapter().Interact' in combat and 'InteractionRequest(' in combat,
 'combat no hells gate event code': 'npcSnapshot.eventCode == 988u' not in combat and 'smMonsterInfo.EventCode == 988' not in combat,
 'combat no direct hells gate open': 'cHg.SendOpenNpc' not in combat,
}
failed=[k for k,v in checks.items() if not v]
if failed:
 print('interaction service boundary v1: FAIL: '+', '.join(failed), file=sys.stderr); sys.exit(1)
print('interaction service boundary v1: PASS')
