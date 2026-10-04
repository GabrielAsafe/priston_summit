#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[1]
red=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='replace')
if 'CharacterSelectionService().Authorize' not in red:
    raise SystemExit('character selection handler is not using V2 authorization service')
if 'lpPlayInfo->getSecurity() < AccountTypes::SEC_GAMEMASTER && rsCheckAccountChar' in red:
    raise SystemExit('legacy handler still owns character ownership/bypass decision')
if 'SendRecordDataToClient(lpPlayInfo, lpTransCharCommand->szName, lpsmSock' not in red:
    raise SystemExit('PATCH 99 transient record socket invariant changed')
print('character selection service validator: PASS')
