#!/usr/bin/env python3
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[1]
handler_path = root / 'Shared/server/legacy_runtime/OnSever_legacy_server.inl'
policy_path = root / 'Shared/security/v2/SecurityPolicy.h'
adapter_path = root / 'SrcServer/src/systems/security/v2/legacy/LegacySecurityEnforcementAdapter.h'

s = handler_path.read_text(encoding='utf-8', errors='replace')
policy = policy_path.read_text(encoding='utf-8', errors='replace')
adapter = adapter_path.read_text(encoding='utf-8', errors='replace')
active = s.split('bool handlePlayerLoginOpcode(smTHREADSOCK* SockInfo)', 1)[1].split('/*bool handlePlayerLoginOpcode', 1)[0]

checks = {
    'v2 validation include': 'authentication/v2/LoginRequestValidation.h' in s,
    'v2 validation invoked': 'LoginRequestValidationService loginValidationService' in active,
    'legacy Joi login schema retired': 'JoiString account = JoiString::build().maximum(ACCOUNT_SIZE - 1)' not in active,
    'invalid login emits structured finding': 'SecurityFindingCode::MalformedLoginRequest' in active,
    'invalid login delegates enforcement': 'ServerLegacySecurityEnforcementAdapter().Apply(' in active,
    'legacy-equivalent policy escalates malformed login': (
        'SecurityFindingCode::MalformedLoginRequest' in policy and
        'SecurityEnforcementAction::DisconnectAndEscalate' in policy
    ),
    'enforcement still records attack': 'recordAttack(player, auditText)' in adapter,
    'enforcement still disconnects': 'DisconnectUser(player->lpsmSock)' in adapter,
    'enforcement still escalates firewall': 'banFirewall(player)' in adapter,
}
failed = [k for k, v in checks.items() if not v]
if failed:
    print('FAIL: ' + ', '.join(failed), file=sys.stderr)
    sys.exit(1)
print('login request policy v2 validator: PASS')
