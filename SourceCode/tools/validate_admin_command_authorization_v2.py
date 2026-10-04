#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[1]
red=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='replace')
service=(root/'Shared/admin/v2/AdminCommandAuthorization.h').read_text(errors='replace')
if red.count('AdminCommandAuthorizationService::Allows') < 4:
    raise SystemExit('not all VIP/moderator/GM/admin command tier gates use V2 authorization')
legacy_gate_patterns=(
    'if (lpPlayInfo->getSecurity() < AccountTypes::SEC_VIP)\n\t\t\treturn FALSE;',
    'if (lpPlayInfo->getSecurity() < AccountTypes::SEC_MODERATOR)\n\t\t\treturn FALSE;',
    'if(lpPlayInfo->getSecurity() < AccountTypes::SEC_GAMEMASTER)\n\t\treturn FALSE;',
    'if(lpPlayInfo->getSecurity() < AccountTypes::SEC_ADMINISTRATOR)\n\t\treturn FALSE;',
)
for legacy in legacy_gate_patterns:
    if legacy in red:
        raise SystemExit(f'legacy command tier gate remains: {legacy.splitlines()[0]}')
if 'actualSecurityLevel >= request.requiredSecurityLevel' not in service:
    raise SystemExit('authorization service no longer preserves ordinal security policy')
print('admin command authorization validator: PASS')
