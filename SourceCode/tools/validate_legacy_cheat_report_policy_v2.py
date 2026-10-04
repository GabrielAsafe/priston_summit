#!/usr/bin/env python3
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[1]
legacy = (root / 'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='replace')
policy = (root / 'Shared/security/v2/LegacyCheatReportPolicy.h').read_text(errors='replace')
adapter = (root / 'SrcServer/src/systems/security/v2/legacy/LegacyCheatReportEnforcementAdapter.h').read_text(errors='replace')

checks = {
    'handler delegates policy': 'LegacyCheatReportPolicy cheatPolicy' in legacy,
    'handler delegates enforcement': 'ServerLegacyCheatReportEnforcementAdapter().Apply' in legacy,
    'legacy handler no Banned flag': 'BOOL Banned = FALSE;' not in legacy[legacy.find('case OpCode::FIND_CHEAT:'):legacy.find('case OpCode::FIND_CHEAT:') + 5000],
    'policy preserves six ban ids': all(f'case {n}u:' in policy for n in (8720, 8954, 8955, 8957, 8960, 8961)),
    'wrong version preserved no action': 'case 8962u:' in policy and 'no audit' in policy,
    'privileged exemption centralized': 'if(privilegedAccount)' in policy,
    'adapter owns account ban': 'banAccount(player)' in adapter,
    'adapter owns audit': 'recordCheatLog(player, auditText)' in adapter,
    'adapter owns ip ban': 'BannedIP(player)' in adapter,
    'adapter owns disconnect': 'DisconnectUser(player->lpsmSock)' in adapter,
}
failed = [name for name, ok in checks.items() if not ok]
if failed:
    for name in failed:
        print(f'FAIL: {name}', file=sys.stderr)
    raise SystemExit(1)
print('legacy cheat report policy v2: PASS')
