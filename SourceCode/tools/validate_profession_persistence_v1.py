#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
types=(root/'Shared/gameplay/professions/v2/ProfessionTypes.h').read_text()
port=(root/'Shared/gameplay/professions/v2/IProfessionRepository.h').read_text()
service=(root/'Shared/gameplay/professions/v2/ProfessionService.h').read_text()
repo=(root/'SrcServer/src/systems/gameplay/professions/v2/SQLiteProfessionRepository.cpp').read_text()
runtime=(root/'SrcServer/src/systems/gameplay/professions/v2/ProfessionRuntime.cpp').read_text()
checks={
 'types split':'enum class ProfessionType' in types and 'struct ProfessionProgress' in types,
 'repository port':'class IProfessionRepository' in port and 'LoadPlayer' in port and 'SaveProgress' in port,
 'green no sql':all(x not in port+service for x in ('SQLConnection','DATABASEID_ServerDB','sqlite3','rsPLAYINFO')),
 'service repository injection':'explicit ProfessionService(IProfessionRepository* repository = 0' in service,
 'lazy load':'EnsureLoaded(playerName)' in service and 'repository_->LoadPlayer' in service,
 'persist mutation':'repository_->SaveProgress' in service,
 'overflow guard':'std::numeric_limits<std::uint64_t>::max() - amount' in service,
 'sqlite schema':'CREATE TABLE IF NOT EXISTS ServerDB__ProfessionProgress' in repo,
 'sqlite upsert':'ON CONFLICT(PlayerKey,ProfessionType) DO UPDATE' in repo,
 'runtime wired':'static ProfessionService service(&ServerProfessionRepository()' in runtime,
}
for k,v in checks.items(): print(('PASS' if v else 'FAIL')+': '+k)
if not all(checks.values()): sys.exit(1)
print('profession persistence v1: PASS (repository port + lazy load + SQLite save wired into server runtime)')
