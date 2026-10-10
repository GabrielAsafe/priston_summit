#!/usr/bin/env bash
set -u

ROOT="${1:-.}"
cd "$ROOT" || exit 1

FILE="SourceCode/Shared/server/legacy_runtime/OnSever_legacy_server.inl"

if [ ! -f "$FILE" ]; then
    echo "[FAIL] missing: $FILE"
    exit 1
fi

STAMP="$(date +%Y%m%d-%H%M%S)"
BACKUP=".summit_native_onserver_backup_$STAMP"
mkdir -p "$BACKUP/$(dirname "$FILE")"
cp -a "$FILE" "$BACKUP/$FILE"
echo "[OK] backup: $BACKUP"

python3 - "$FILE" <<'PY'
from pathlib import Path
import sys

p = Path(sys.argv[1])
s = p.read_text(encoding="utf-8", errors="surrogateescape")

anchor = '#include "systems/commerce/v2/legacy/LegacyTimeCreditWalletAdapter.h"\n'
early_includes = [
    '#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"',
    '#include "systems/inventory/v2/legacy/LegacyInventoryRuntime.h"',
    '#include "systems/social/party/v2/legacy/LegacyPartyCommandsAdapter.h"',
    '#include "systems/social/clan/v2/legacy/LegacyClanQueryAdapter.h"',
    '#include "systems/social/clan/v2/legacy/LegacyClanMembershipAdapter.h"',
]

if anchor not in s:
    raise SystemExit("[FAIL] expected include anchor not found")

# These headers may also occur later in the monolith. We intentionally ensure
# they are visible before the helper block that starts near the top of the file.
prefix_end = s.index(anchor) + len(anchor)
prefix = s[:prefix_end]
suffix = s[prefix_end:]
missing_early = [inc for inc in early_includes if inc not in prefix]
if missing_early:
    s = prefix + ''.join(inc + '\n' for inc in missing_early) + suffix

duplicate = (
    'inline gameplay::PlayerId SummitOnServerPlayerId(rsPLAYINFO* player)\n'
    '{\n'
    '\treturn gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry().IdOf(player);\n'
    '}\n\n'
)
if duplicate in s:
    s = s.replace(duplicate, '', 1)

bad = (
    'return lpPlayInfo_LoginServer[dwSerToLogin_Num++]->'
    'SendOnServerLegacySocket(lpsmSock, szBuff, size);'
)
good = (
    'return SendOnServerLegacySocket('
    'lpPlayInfo_LoginServer[dwSerToLogin_Num++]->lpsmSock, szBuff, size);'
)
if bad in s:
    s = s.replace(bad, good, 1)

required = [
    'static gameplay::PlayerId SummitOnServerPlayerId(rsPLAYINFO* player)',
    'gameplay::inventory::legacy::ServerLegacyInventoryRuntime().Add(',
    'gameplay::social::party::legacy::ServerLegacyPartyCommandsAdapter().ClearMembership(',
    'gameplay::social::clan::legacy::ServerLegacyClanQueryAdapter().TryGetMembership(',
    'gameplay::social::clan::legacy::ServerLegacyClanMembershipAdapter()',
    good,
]
missing = [token for token in required if token not in s]
if missing:
    raise SystemExit("[FAIL] expected post-fix tokens missing: " + repr(missing))

if bad in s:
    raise SystemExit("[FAIL] malformed legacy socket member call still present")

if s.count('SummitOnServerPlayerId(rsPLAYINFO* player)') != 1:
    raise SystemExit(
        "[FAIL] SummitOnServerPlayerId definition count is not 1: "
        + str(s.count('SummitOnServerPlayerId(rsPLAYINFO* player)'))
    )

p.write_text(s, encoding="utf-8", errors="surrogateescape")
print("[OK] OnSever_legacy_server.inl native compile hotfix applied")
PY

echo
echo "=== SANITY ==="
grep -n \
  -e 'LegacyInventoryRuntime.h' \
  -e 'LegacyPartyCommandsAdapter.h' \
  -e 'LegacyClanQueryAdapter.h' \
  -e 'LegacyClanMembershipAdapter.h' \
  -e 'SummitOnServerPlayerId(rsPLAYINFO' \
  -e 'SendOnServerLegacySocket(lpPlayInfo_LoginServer' \
  "$FILE"

echo
echo "=== RELEVANT GATES ==="
cd SourceCode || exit 1

python3 tools/validate_onserver_inventory_authority_v2.py || exit 1
python3 tools/validate_party_membership_mutation_ownership_v2.py || exit 1
python3 tools/validate_clan_membership_mutation_ownership_v1.py || exit 1
python3 tools/validate_clan_query_ownership_v2.py || exit 1
python3 tools/validate_legacy_monolith_authority_residual_v1.py || exit 1
python3 tools/architecture/validate_zero_legacy_debt.py || exit 1

cd .. || exit 1

echo
echo "=== DIFF STAT ==="
git diff --stat -- "$FILE"
echo
git diff -- "$FILE"

echo
echo "[PASS] OnServer hotfix applied and ownership gates passed"
echo "[INFO] do not commit until native client+server build reaches FAIL : 0"
echo "[INFO] backup: $BACKUP"
