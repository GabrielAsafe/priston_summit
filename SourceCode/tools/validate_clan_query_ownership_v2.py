#!/usr/bin/env python3
"""Prevent OnServer gameplay from bypassing the V2 clan query boundary."""
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
ONSERVER = ROOT / "Shared/server/legacy_runtime/OnSever_legacy_server.inl"
text = ONSERVER.read_text(encoding="utf-8")

required = [
    "ServerLegacyClanQueryAdapter().TryGetMembership",
    "SummitOnServerClanMembership",
    "SummitOnServerClanCode",
    "SummitOnServerClanMark",
]
missing = [token for token in required if token not in text]
if missing:
    for token in missing:
        print(f"FAIL: missing clan query boundary token: {token}", file=sys.stderr)
    sys.exit(1)

# Direct player clan reads are forbidden in gameplay.  A small set of writes is
# still legitimate legacy shell work: connection-slot initialization and
# character/clan hydration from persistence/network sync.  Reads on the RHS of
# those assignment lines are part of that hydration operation.
player_field = re.compile(
    r"(?:lpPlayInfo|lprsPlayInfo|rsPlayInfo\[[^\]]+\])(?:->|\.)"
    r"(?:dwClanCode|smCharInfo\.ClassClan)"
)
write_lhs = re.compile(
    r"^\s*(?:lpPlayInfo|lprsPlayInfo|rsPlayInfo\[[^\]]+\])(?:->|\.)"
    r"(?:dwClanCode|smCharInfo\.ClassClan)\s*=(?!=)"
)

unauthorized = []
shell_writes = 0
for line_no, raw in enumerate(text.splitlines(), 1):
    code = raw.split("//", 1)[0]
    if not player_field.search(code):
        continue
    if write_lhs.search(code):
        shell_writes += 1
        continue
    unauthorized.append(f"{line_no}: {code.strip()}")

if unauthorized:
    print("clan query ownership v2: FAIL", file=sys.stderr)
    for item in unauthorized:
        print(f"  direct gameplay clan read: {item}", file=sys.stderr)
    sys.exit(1)

helper_uses = text.count("SummitOnServerClanCode(") + text.count("SummitOnServerClanMark(")
if helper_uses < 12:
    print(f"clan query ownership v2: FAIL (expected broad query delegation, found {helper_uses} helper uses)", file=sys.stderr)
    sys.exit(1)

print(
    "clan query ownership v2: PASS "
    f"(query_helper_uses={helper_uses}, shell_hydration_writes={shell_writes}, direct_gameplay_reads=0)"
)
