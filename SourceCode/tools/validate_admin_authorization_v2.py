#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[1]
h=(root/'SrcServer/src/systems/admin/game_masters/GameMasters.h').read_text(errors='replace')
cpp=(root/'SrcServer/src/systems/admin/game_masters/GameMasters.cpp').read_text(errors='replace')
for req in ('AdminAuthorizationService authorization', 'rebuildAuthorization'):
    if req not in h: raise SystemExit(f'missing admin adapter boundary: {req}')
for req in ('authorization.Authenticate', 'authorization.ResolveActiveSecurity', 'authorization.ReplaceAll'):
    if req not in cpp: raise SystemExit(f'missing admin authorization consumption: {req}')
# Credential/role traversal should live in GREEN, not duplicate in the legacy adapter methods.
segment=cpp[cpp.index('AccountTypes GameMasters::getGameMaster'):cpp.index('void GameMasters::print')]
if 'for(' in segment or 'for (' in segment:
    raise SystemExit('legacy GameMasters still owns credential traversal policy')
print('admin authorization validator: PASS')
