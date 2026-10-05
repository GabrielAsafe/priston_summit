#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[2]
p=root/'SrcServer/src/systems/gameplay/quests/server/cQuestHandler.cpp'
t=p.read_text(encoding='utf-8',errors='replace')
start=t.find('bool cQUESTHANDLER::KillMonster')
end=t.find('bool cQUESTHANDLER::updateKilledMonsterOfPartyMember', start)
if start < 0 or end < 0:
    print('PATCH 180 missing cQUESTHANDLER::KillMonster block', file=sys.stderr); sys.exit(1)
kill=t[start:end]
required=(
    'it->killed >= it->count',
    'ServerLegacyQuestRepository().CompleteObjectives(',
    'GetQuestInfo(',
)
missing=[x for x in required if x not in kill]
if missing:
    print('PATCH 180 missing: '+', '.join(missing),file=sys.stderr); sys.exit(1)
if kill.index('CompleteObjectives(') > kill.index('GetQuestInfo('):
    print('PATCH 180 completion must precede presentation refresh',file=sys.stderr); sys.exit(1)
print('PATCH 180 gate: OK (cQUESTHANDLER authority)')
