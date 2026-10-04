#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
cpp=(root/'SrcServer/src/systems/progression/class_rank/Rank.cpp').read_text(errors='replace')
repo=(root/'SrcServer/src/systems/progression/class_rank/v2/legacy/LegacyClassRankRepository.h').read_text(errors='replace')
model=(root/'Shared/gameplay/progression/v2/ClassRankReadModel.h').read_text(errors='replace')
start=cpp.index('void Rank::SendRankclassToGame')
body=cpp[start:]
checks={
 'handler delegates read model': 'ClassRankReadService::LoadTop' in body,
 'handler has no SQL': 'SQLConnection::GetConnection' not in body and 'SELECT * FROM [PVP]' not in body,
 'repository owns PVP query': 'SELECT * FROM [PVP]' in repo,
 'repository reads level into int64': 'std::int64_t level = 0' in repo and 'PARAMTYPE_Int64' in repo,
 'legacy packet conversion explicit': 'static_cast<int>(entry.level)' in body,
 'legacy result limit preserved': 'LegacyResultLimit = 51u' in model,
}
failed=[n for n,v in checks.items() if not v]
if failed:
 for n in failed: print('FAIL:',n,file=sys.stderr)
 raise SystemExit(1)
print('class rank read model v2: PASS')
