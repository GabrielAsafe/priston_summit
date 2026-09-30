#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[1]
event=(root/'SrcServer/src/systems/gameplay/events/v2/legacy/LegacyGameplayEventAdapter.cpp').read_text()
adapter=(root/'SrcServer/src/systems/gameplay/quests/v2/legacy/LegacyQuestProgressService.cpp').read_text()
onserver=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_bytes().decode('latin1')
contract=(root/'Shared/gameplay/quests/v2/QuestProgressService.h').read_text()
checks=[
 ('contract exposes progress service', 'class IQuestProgressService' in contract and 'RecordMonsterKill' in contract),
 ('event adapter no quest handler', 'cQuestHandler' not in event and 'cQuestHandler.h' not in event),
 ('event adapter delegates progress', 'ServerLegacyQuestProgressService().RecordMonsterKill' in event),
 ('event IDs preserved', 'event.player, event.monster, event.partyCredit' in event),
 ('legacy adapter owns quest handler', 'cQuestHandler.KillMonster' in adapter),
 ('legacy adapter resolves authenticated player', 'ServerLegacyOnlinePlayerRegistry().Resolve(player)' in adapter),
 ('legacy adapter resolves monster', 'srFindCharFromSerial' in adapter),
 ('party flag preserved', 'partyCredit ? 1 : 0' in adapter),
 ('tier5 contract exposed', 'RecordTier5MonsterKill' in contract and 'std::vector<PlayerId>' in contract),
 ('tier5 legacy handler owned by adapter', 'cQuestHandler.HandleTier5MonsterKill' in adapter),
 ('tier5 damage users resolved by registry', 'resolvedDamageUsers' in adapter and 'Resolve(*it)' in adapter),
 ('onserver no direct tier5 quest handler', 'cQuestHandler.HandleTier5MonsterKill' not in onserver),
 ('onserver delegates tier5 progress', 'ServerLegacyQuestProgressService().RecordTier5MonsterKill' in onserver),
 ('onserver authenticates tier5 participant ids', 'ServerLegacyOnlinePlayerRegistry().IdOf(lpDamagePlayInfo[cnt])' in onserver),
]
failed=[]
for label,ok in checks:
 print(('PASS: ' if ok else 'FAIL: ')+label)
 if not ok: failed.append(label)
if failed: raise SystemExit(1)
print('quest progress boundary: PASS')
