#!/usr/bin/env bash
set -u

ROOT="${1:-.}"
cd "$ROOT" || exit 1

FILE="SourceCode/SrcGame/src/systems/social/party/ui/cPartyWindow.h"

if [ ! -f "$FILE" ]; then
    echo "[FAIL] missing: $FILE"
    exit 1
fi

STAMP="$(date +%Y%m%d-%H%M%S)"
BACKUP=".summit_native_party_header_backup_$STAMP"
mkdir -p "$BACKUP/$(dirname "$FILE")"
cp -a "$FILE" "$BACKUP/$FILE"
echo "[OK] backup: $BACKUP"

python3 - "$FILE" <<'PY'
from pathlib import Path
import sys

p = Path(sys.argv[1])
s = p.read_text(encoding="utf-8", errors="surrogateescape")

# Idempotent include/self-containment fix.
if '#include <vector>' not in s:
    needle = '#include <map>\n'
    if needle not in s:
        raise SystemExit('[FAIL] expected #include <map> anchor not found')
    s = s.replace(
        needle,
        '#include <map>\n'
        '#include <vector>\n\n'
        '#include "smPacket.h"\n\n'
        'class smCHAR;\n',
        1
    )
else:
    if '#include "smPacket.h"' not in s:
        anchor = '#include <vector>\n'
        if anchor not in s:
            raise SystemExit('[FAIL] vector include state is unexpected')
        s = s.replace(anchor, anchor + '\n#include "smPacket.h"\n', 1)
    if 'class smCHAR;' not in s:
        anchor = '#include "smPacket.h"\n'
        s = s.replace(anchor, anchor + '\nclass smCHAR;\n', 1)

old = '\t\tvector<sMENUOPT*> vOption;'
new = '\t\tstd::vector<sMENUOPT*> vOption;'
if old in s:
    s = s.replace(old, new, 1)
elif new not in s:
    # Also tolerate spaces instead of tabs.
    old2 = '        vector<sMENUOPT*> vOption;'
    if old2 in s:
        s = s.replace(old2, '        std::vector<sMENUOPT*> vOption;', 1)
    else:
        raise SystemExit('[FAIL] vOption declaration not found in expected form')

required = [
    '#include <vector>',
    '#include "smPacket.h"',
    'class smCHAR;',
    'std::vector<sMENUOPT*> vOption;',
    '_PARTY_USER Members[6];',
]
missing = [x for x in required if x not in s]
if missing:
    raise SystemExit('[FAIL] missing expected tokens after patch: ' + ', '.join(missing))

p.write_text(s, encoding="utf-8", errors="surrogateescape")
print('[OK] cPartyWindow.h made self-contained')
PY

echo
echo "=== PATCHED HEADER ==="
sed -n '1,95p' "$FILE"

echo
echo "=== DIFF ==="
git diff -- "$FILE"

echo
echo "[PASS] native party header hotfix staged for build validation"
echo "[INFO] do not commit until compboth.sh passes"
