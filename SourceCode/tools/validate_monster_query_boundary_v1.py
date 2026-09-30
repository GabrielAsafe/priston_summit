#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
contract=(root/'Shared/gameplay/monsters/v2/MonsterQuery.h').read_text(errors='replace')
header=(root/'SrcServer/src/systems/gameplay/monsters/v2/legacy/LegacyMonsterAdapter.h').read_text(errors='replace')
adapter=(root/'SrcServer/src/systems/gameplay/monsters/v2/legacy/LegacyMonsterAdapter.cpp').read_text(errors='replace')
executor=(root/'SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp').read_text(errors='replace')
checks={
 'green monster snapshot': all(x in contract for x in ('enum class MonsterFamily','struct MonsterSnapshot','class IMonsterQuery','TryGetMonster(EntityId')),
 'adapter exposes query and commands': 'public IMonsterCommands, public IMonsterQuery' in header and 'TryGetMonster(EntityId entity' in header,
 'adapter resolves runtime entity': 'srFindCharFromSerial' in adapter and 'snapshot.family = ToMonsterFamily' in adapter,
 'adapter maps core monster fields': all(x in adapter for x in ('snapshot.life =','snapshot.maxLife =','snapshot.bionicResistance =','snapshot.controlled =')),
 'companion consumes monster query': executor.count('ServerLegacyMonsterAdapter().TryGetMonster') >= 3,
 'holy incantation uses family snapshot': 'monster.family != monsters::MonsterFamily::Undead' in executor,
 'extinction uses snapshot life': '(monster.life * Extinction_Amount[p]) / 100' in executor,
}
failed=[k for k,v in checks.items() if not v]
if failed:
 print('monster query boundary v1: FAIL: '+', '.join(failed), file=sys.stderr); sys.exit(1)
print('monster query boundary v1: PASS')
