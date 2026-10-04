#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
legacy=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='replace')
policy=(root/'Shared/authentication/v2/DuplicateLoginPolicy.h').read_text(errors='replace')
start=legacy.find('case LOAD_DLL_SQL:')
end=legacy.find('int rsLogAccount(', start)
segment=legacy[start:end]
checks={
 'policy used in both duplicate checks': segment.count('DuplicateLoginPolicy::Evaluate') == 2,
 'legacy stale arithmetic removed': segment.count('dwLastRecvTime + 60 * 1000') == 0,
 'stale retirement preserved': segment.count('RetireStaleAndRejectNewSession') == 2,
 'legacy reject code preserved': 'val = -4;' in segment and 'cbCnt = -4;' in legacy,
 'timeout owned by policy': 'StaleAfterMilliseconds = 60u * 1000u' in policy,
}
failed=[n for n,v in checks.items() if not v]
if failed:
 for n in failed: print('FAIL:',n,file=sys.stderr)
 raise SystemExit(1)
print('duplicate login policy v2: PASS')
