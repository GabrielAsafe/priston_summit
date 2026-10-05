#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[2]
iface=(root/'Shared/gameplay/quests/v2/QuestProgressService.h').read_text(encoding='utf-8',errors='replace')
h=(root/'SrcServer/src/systems/gameplay/quests/v2/legacy/LegacyQuestProgressService.h').read_text(encoding='utf-8',errors='replace')
cpp=(root/'SrcServer/src/systems/gameplay/quests/v2/legacy/LegacyQuestProgressService.cpp').read_text(encoding='utf-8',errors='replace')
handler=(root/'SrcServer/src/systems/gameplay/quests/server/cQuestHandler.cpp').read_text(encoding='utf-8',errors='replace')
errors=[]
for name in ('RecordMonsterKill','RecordTier5MonsterKill'):
    if name not in iface or name not in h or name not in cpp:
        errors.append(f'missing required progress API: {name}')
for stale in ('StartProgress(', 'CancelProgressVariant(', 'ClaimReward(', 'HasPartyObjectiveTarget('):
    # They may exist elsewhere, but must not be declared as interface overrides here.
    if stale in h:
        errors.append(f'stale non-interface override in LegacyQuestProgressService.h: {stale}')
if 'Tier5QuestService.h' in cpp:
    errors.append('LegacyQuestProgressService.cpp still depends on unavailable Tier5QuestService.h')
if 'cQuestHandler.KillMonster(' not in cpp:
    errors.append('RecordMonsterKill no longer delegates to branch-authoritative cQuestHandler.KillMonster')
if 'cQuestHandler.HandleTier5MonsterKill(' not in cpp:
    errors.append('RecordTier5MonsterKill no longer delegates to existing Tier5 handler')
start=handler.find('bool cQUESTHANDLER::KillMonster')
end=handler.find('bool cQUESTHANDLER::updateKilledMonsterOfPartyMember', start)
block=handler[start:end] if start >= 0 and end >= 0 else ''
if 'ServerLegacyQuestRepository().CompleteObjectives(' not in block or 'it->killed >= it->count' not in block:
    errors.append('PATCH 180 completion handoff missing from cQUESTHANDLER::KillMonster')
if errors:
    for e in errors: print('PATCH 198 ERROR: '+e,file=sys.stderr)
    sys.exit(1)
print('PATCH 198 gate: OK — quest progress adapter matches branch interface and PATCH 180 completion remains active')
