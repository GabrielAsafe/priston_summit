#!/usr/bin/env python3
from pathlib import Path
import re, sys
root=Path(__file__).resolve().parents[1]
legacy=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='replace')
adapter=(root/'SrcServer/src/systems/admin/v2/legacy/LegacyAdminAuditAdapter.h').read_text(errors='replace')
# Exclude declaration/definition: call sites use the exact lpPlayInfo prefix.
checks={
 'red has no direct audit calls': 'rsRecordAdminCommand(lpPlayInfo, ' not in legacy,
 'adapter is used broadly': legacy.count('ServerLegacyAdminAuditAdapter().Record') >= 45,
 'adapter delegates legacy recorder': 'rsRecordAdminCommand(' in adapter,
 'caravan audited': re.search(r'rsCompString\("/caravan".*?ServerLegacyAdminAuditAdapter\(\)\.Record', legacy, re.S) is not None,
 'global off audited': re.search(r'rsCompString\("/GlobalOFF".*?ServerLegacyAdminAuditAdapter\(\)\.Record', legacy, re.S) is not None,
 'global on audited': re.search(r'rsCompString\("/GlobalON".*?ServerLegacyAdminAuditAdapter\(\)\.Record', legacy, re.S) is not None,
 'arena monster audited': re.search(r'rsCompString\("/arena_monster".*?ServerLegacyAdminAuditAdapter\(\)\.Record', legacy, re.S) is not None,
}
failed=[n for n,v in checks.items() if not v]
if failed:
 for n in failed: print('FAIL:',n,file=sys.stderr)
 raise SystemExit(1)
print('admin audit adapter v2: PASS')
