#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[1]
red=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='replace')
adapter=(root/'SrcServer/src/systems/security/v2/legacy/LegacySecurityEnforcementAdapter.h').read_text(errors='replace')
if 'ServerLegacySecurityEnforcementAdapter().Apply' not in red:
    raise SystemExit('login security policy result is not delegated to enforcement adapter')
segment=red[red.index('finding.code = security::v2::SecurityFindingCode::MalformedLoginRequest'):]
segment=segment[:segment.index('return false;')]
for forbidden in ('banFirewall(playInfo)', 'DisconnectUser(playInfo->lpsmSock)', 'recordAttack(playInfo'):
    if forbidden in segment:
        raise SystemExit(f'direct enforcement remains in migrated login branch: {forbidden}')
for required in ('recordAttack(player, auditText)', 'banFirewall(player)', 'DisconnectUser(player->lpsmSock)'):
    if required not in adapter:
        raise SystemExit(f'legacy enforcement adapter missing action: {required}')
print('security enforcement adapter validator: PASS')
