#!/usr/bin/env python3
from pathlib import Path
import sqlite3, sys

ROOT = Path(__file__).resolve().parents[1]
checks=[]

def req(path, needle, label):
    p=ROOT/path
    ok=p.exists() and needle in p.read_text('utf-8',errors='ignore')
    checks.append((ok,label))
    if not ok: print(f'FAIL: {label}: {path} missing {needle!r}')

req('Database/migrations/044_gathering_mining_runtime.sql','GatheringResourceDefinition','resource content schema')
req('Database/migrations/044_gathering_mining_runtime.sql','GatheringNodeDefinition','persistent node schema')
req('Database/migrations/044_gathering_mining_runtime.sql','GatheringPlayerInventory','persistent resource inventory')
req('SrcServer/src/systems/persistence/database/sqlite/SQLiteConnection.cpp','044_gathering_mining_runtime.sql','migration 044 registered')
req('Database/migrations/045_gathering_serverdb_namespace_fix.sql','ServerDB__GatheringResourceDefinition','gathering namespace repair migration')
req('SrcServer/src/systems/persistence/database/sqlite/SQLiteConnection.cpp','045_gathering_serverdb_namespace_fix.sql','migration 045 registered')
req('SrcServer/src/platform/windows/application/Winmain.cpp','InitializeServerGatheringRuntime','runtime starts with server')
req('Shared/server/legacy_runtime/OnSever_legacy_server.inl','/mine','mining command surface')
req('Shared/server/legacy_runtime/OnSever_legacy_server.inl','/gather','gathering command surface')
req('Shared/server/legacy_runtime/OnSever_legacy_server.inl','AddServerGatheringNode','persistent node placement')
req('SrcServer/src/systems/gameplay/gathering/v2/GatheringRuntime.cpp','AddInventory','harvest persists base resources')
req('SrcServer/src/systems/gameplay/gathering/v2/GatheringRuntime.cpp','gathering.harvest','gatherer activity progresses Covenant')
req('SrcServer/src/systems/gameplay/mining/v2/MiningInteractionRuntime.cpp','ServerGatheringRepository().AddInventory','mining persists base resources')
req('SrcGame/src/systems/ui/v2/pages/CovenantsPage.cpp','Minerar proximo','mining UI action')
req('SrcGame/src/systems/ui/v2/pages/CovenantsPage.cpp','Coletar proximo','gathering UI action')
req('SrcGame/src/systems/ui/v2/pages/CovenantsPage.cpp','RECURSOS COLETADOS','resource inventory visible in UI')
req('Shared/gameplay/covenants/v2/CovenantUIProtocol.h','COVENANT_UI_MAX_RESOURCES','resource inventory crosses UI protocol')

# Verify migration can be applied repeatedly with a conservative sqlite engine.
sql=(ROOT/'Database/migrations/044_gathering_mining_runtime.sql').read_text('utf-8')
conn=sqlite3.connect(':memory:')
try:
    conn.executescript(sql)
    conn.executescript(sql)
    resource_count=conn.execute('SELECT COUNT(*) FROM GatheringResourceDefinition').fetchone()[0]
    if resource_count >= 6:
        checks.append((True,'migration 044 seeds starter resource catalog'))
    else:
        checks.append((False,'migration 044 seeds starter resource catalog'))
        print('FAIL: expected at least 6 starter resources')
    node_count=conn.execute('SELECT COUNT(*) FROM GatheringNodeDefinition').fetchone()[0]
    ok=node_count == 0
    checks.append((ok,'migration does not guess production node coordinates'))
    if not ok: print('FAIL: migration 044 must not seed guessed node coordinates')
finally:
    conn.close()

failed=[label for ok,label in checks if not ok]
if failed:
    print(f'Gathering/Mining V1 validation: FAIL ({len(failed)}/{len(checks)})')
    sys.exit(1)
print(f'Gathering/Mining V1 validation: PASS ({len(checks)}/{len(checks)})')
