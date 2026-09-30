#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
event=(root/'SrcServer/src/systems/gameplay/events/v2/MonsterKilledEvent.h').read_text()
runtime_h=(root/'SrcServer/src/systems/gameplay/events/v2/GameplayEventRuntime.h').read_text()
runtime_cpp=(root/'SrcServer/src/systems/gameplay/events/v2/GameplayEventRuntime.cpp').read_text()
legacy=(root/'SrcServer/src/systems/gameplay/events/v2/legacy/LegacyGameplayEventAdapter.cpp').read_text()
server=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text()
party=(root/'SrcServer/src/core/server/legacy_runtime/PartySever.cpp').read_text()
allow=(root/'tools/architecture/legacy_boundary_allowlist.txt').read_text()
checks={
 'event uses ids':'PlayerId player;' in event and 'EntityId monster;' in event,
 'event snapshots boss/name/code/area':all(x in event for x in ('bool monsterBoss;','std::string playerName;','std::string monsterName;','std::uint32_t monsterCode;','int area;')),
 'event legacy-clean':all(x not in event for x in ('rsPLAYINFO','smCHAR','smd3d.h','character.h','cQuestHandler')),
 'runtime public event-only':'PublishMonsterKilled(const MonsterKilledEvent& event)' in runtime_h,
 'runtime legacy-clean':all(x not in runtime_h+runtime_cpp for x in ('rsPLAYINFO','smCHAR','smd3d.h','character.h','cQuestHandler')),
 'covenant boss uses snapshot':'if (event.monsterBoss)' in runtime_cpp and 'event.monster.value' in runtime_cpp,
 'legacy adapter translates ids':'IdOf(playerValue)' in legacy and 'EntityId monsterId' in legacy,
 'quest progress remains subscriber':'Subscribe<MonsterKilledEvent>' in legacy and 'ServerLegacyQuestProgressService().RecordMonsterKill' in legacy and 'cQuestHandler' not in legacy,
 'server caller uses amber':'ServerLegacyGameplayEventAdapter().PublishMonsterKilled(lprsPlayInfo, lpChar, false)' in server,
 'party callers use amber':party.count('ServerLegacyGameplayEventAdapter().PublishMonsterKilled') >= 2,
 'event allowlist empty':'SrcServer/src/systems/gameplay/events/v2/' not in allow,
}
for k,v in checks.items(): print(('PASS' if v else 'FAIL')+': '+k)
if not all(checks.values()): sys.exit(1)
print('monster killed event boundary v1: PASS (event/runtime pointer-free; Quest progress subscriber isolated behind service)')
