#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
s=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(encoding='utf-8',errors='replace')
active=s.split('bool handlePlayerLoginOpcode(smTHREADSOCK* SockInfo)',1)[1].split('/*bool handlePlayerLoginOpcode',1)[0]
checks={
 'finding model': (root/'Shared/security/v2/SecurityFinding.h').exists(),
 'policy contract': (root/'Shared/security/v2/SecurityPolicy.h').exists(),
 'login produces finding': 'SecurityFindingCode::MalformedLoginRequest' in active,
 'policy owns enforcement decision': 'securityPolicy.Evaluate(finding)' in active,
 'enforcement delegated to adapter': 'ServerLegacySecurityEnforcementAdapter().Apply' in active,
 'legacy enforcement adapter exists': (root/'SrcServer/src/systems/security/v2/legacy/LegacySecurityEnforcementAdapter.h').exists(),
}
failed=[k for k,v in checks.items() if not v]
if failed:
 print('FAIL: '+', '.join(failed),file=sys.stderr);sys.exit(1)
print('security policy/enforcement validator: PASS')
