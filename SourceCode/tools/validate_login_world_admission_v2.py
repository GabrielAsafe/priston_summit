#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[1]
reg=(root/'SrcServer/src/systems/authentication/v2/legacy/LegacyLoginSessionRegistry.h').read_text(errors='replace')
red=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='replace')
for req in ('RequestWorldAdmission', 'AcceptWorldAdmission', 'BeginReconnect'):
    if req not in reg: raise SystemExit(f'login session registry missing lifecycle operation: {req}')
case=red.split('case OpCode::OPCODE_PLAYERINFO:',1)[1].split('case OpCode::OPCODE_PLAYERINFO2:',1)[0]
if 'RequestWorldAdmission(lpPlayInfo)' not in case or 'AcceptWorldAdmission(lpPlayInfo)' not in case:
    raise SystemExit('OPCODE_PLAYERINFO does not bracket world admission in session registry')
if case.index('RequestWorldAdmission(lpPlayInfo)') > case.index('TransInfoFlag = TRUE'):
    raise SystemExit('world admission request occurs too late')
if case.index('AcceptWorldAdmission(lpPlayInfo)') < case.index('TransInfoFlag = TRUE'):
    raise SystemExit('world admission accepted before legacy admission flag')
print('login world admission validator: PASS')
