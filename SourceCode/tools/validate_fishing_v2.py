#!/usr/bin/env python3
from pathlib import Path
import re, sqlite3, sys
ROOT=Path(__file__).resolve().parents[1]
checks=[]
def req(path, needle, label):
    p=ROOT/path; text=p.read_text('utf-8',errors='ignore') if p.exists() else ''
    ok=needle in text; checks.append((ok,label))
    if not ok: print(f'FAIL: {label}: {path} missing {needle!r}')
req('Shared/gameplay/professions/v2/ProfessionTypes.h','Fisher','Fisher V2 identity exists')
req('Shared/gameplay/gathering/v2/GatheringTypes.h','Fishing','Fishing gathering kind exists')
req('SrcServer/src/systems/gameplay/fishing/v2/FishingService.cpp','ProfessionType::Fisher','Fishing consumes Fisher V2 profession')
req('SrcServer/src/systems/gameplay/fishing/v2/FishingService.cpp','AddExperience','successful catch progresses Fisher XP')
req('SrcServer/src/systems/gameplay/fishing/v2/FishingRuntime.cpp','AddInventory','catch persists resource inventory')
req('SrcServer/src/systems/network/v2/legacy/LegacyModernChatIngressAdapter.inl','/fish','Fishing player ingress exists')
req('SrcServer/src/systems/network/v2/legacy/LegacyModernChatIngressAdapter.inl','FishNodeForPlayer','Fishing ingress delegates to V2 runtime')
req('SrcServer/src/systems/network/v2/legacy/LegacyModernChatIngressAdapter.inl','GatheringKind::Fishing','nearest-node lookup is Fishing-scoped')
req('SrcServer/src/systems/gameplay/gathering/v2/GatheringRuntime.cpp','ServerFishingService','Gathering bootstrap registers Fishing content')
req('SrcServer/src/systems/persistence/database/sqlite/SQLiteConnection.cpp','046_fishing_vertical.sql','Fishing migration registered')
text=(ROOT/'SrcServer/src/systems/gameplay/fishing/v2/FishingService.cpp').read_text('utf-8',errors='ignore')
job_guess=bool(re.search(r'JOB_CODE', text)) and 'Do not map' not in text
checks.append((not job_guess,'Fishing does not invent a legacy JOB_CODE mapping'))

# Apply migrations 044-046 in the same namespace shape used by the runtime.
conn=sqlite3.connect(':memory:')
try:
    conn.executescript((ROOT/'Database/migrations/044_gathering_mining_runtime.sql').read_text())
    conn.executescript((ROOT/'Database/migrations/045_gathering_serverdb_namespace_fix.sql').read_text())
    sql=(ROOT/'Database/migrations/046_fishing_vertical.sql').read_text()
    conn.executescript(sql); conn.executescript(sql)
    rows=conn.execute("SELECT ResourceId,Kind,RequiredProfession FROM ServerDB__GatheringResourceDefinition WHERE ResourceKey LIKE '%trout' OR ResourceKey LIKE '%carp' OR ResourceKey LIKE '%eel' ORDER BY ResourceId").fetchall()
    ok=len(rows)==3 and all(kind==1 and profession==6 for _,kind,profession in rows)
    checks.append((ok,'Fishing catalog is idempotent and bound to Fisher V2 identity'))
    if not ok: print('FAIL: unexpected Fishing catalog rows', rows)
finally: conn.close()
failed=[x for x in checks if not x[0]]
if failed:
    print(f'Fishing V2 validation: FAIL ({len(failed)}/{len(checks)})'); sys.exit(1)
print(f'Fishing V2 validation: PASS ({len(checks)}/{len(checks)})')
