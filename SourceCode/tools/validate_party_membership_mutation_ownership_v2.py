#!/usr/bin/env python3
"""Enforce V2 ownership of mutable server party membership state.

PartySever remains the private legacy kernel and LegacyPartyCommandsAdapter is
its authorized mutation boundary.  OnServer may initialize an empty connection
slot, but gameplay code must use IPartyCommands instead of writing membership
fields directly.
"""
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
SERVER = ROOT / "SrcServer" / "src"
SHARED_SERVER = ROOT / "Shared" / "server"
KERNEL = (ROOT / "SrcServer/src/core/server/legacy_runtime/PartySever.cpp").resolve()
ADAPTER = (ROOT / "SrcServer/src/systems/social/party/v2/legacy/LegacyPartyCommandsAdapter.cpp").resolve()
ONSERVER = (ROOT / "Shared/server/legacy_runtime/OnSever_legacy_server.inl").resolve()

FIELD_ASSIGNMENT = re.compile(r"(?:->|\.)\s*(?:dwPartyInfo|lpPartyMaster)\s*=(?!=)")
KERNEL_ASSIGNMENT = re.compile(r"\b(?:dwPartyInfo|lpPartyMaster)\s*=(?!=)")
INIT_LINES = {
    "rsPlayInfo[cnt].dwPartyInfo = 0;",
    "rsPlayInfo[cnt].lpPartyMaster = 0;",
}
SKIP_PARTS = {"legacy_oracle", ".git"}

kernel_mutations = 0
adapter_mutations = 0
shell_initialization = 0
unauthorized = []

for base in (SERVER, SHARED_SERVER):
    for path in base.rglob("*"):
        if not path.is_file() or path.suffix.lower() not in {".cpp", ".c", ".h", ".hpp", ".inl"}:
            continue
        if any(part in SKIP_PARTS for part in path.parts):
            continue
        name = path.name.lower()
        if ".before" in name or "_legacy_client" in name:
            continue
        try:
            text = path.read_text(encoding="utf-8")
        except UnicodeDecodeError:
            text = path.read_text(encoding="latin-1")
        resolved = path.resolve()
        for line_no, line in enumerate(text.splitlines(), 1):
            code = line.split("//", 1)[0]
            stripped = code.strip()
            if resolved == KERNEL:
                if KERNEL_ASSIGNMENT.search(code):
                    kernel_mutations += 1
                continue
            if not FIELD_ASSIGNMENT.search(code):
                continue
            if resolved == ADAPTER:
                adapter_mutations += 1
                continue
            if resolved == ONSERVER and stripped in INIT_LINES:
                shell_initialization += 1
                continue
            unauthorized.append(f"{path.relative_to(ROOT)}:{line_no}: {stripped}")

if unauthorized:
    print("party membership mutation ownership v2: FAIL", file=sys.stderr)
    for item in unauthorized:
        print(f"  unauthorized: {item}", file=sys.stderr)
    sys.exit(1)
if adapter_mutations == 0 or kernel_mutations == 0:
    print("party membership mutation ownership v2: FAIL (expected kernel/adapter mutations not found)", file=sys.stderr)
    sys.exit(1)
if shell_initialization != 2:
    print(
        f"party membership mutation ownership v2: FAIL (expected 2 OnServer slot initialization writes, found {shell_initialization})",
        file=sys.stderr,
    )
    sys.exit(1)

print(
    "party membership mutation ownership v2: PASS "
    f"(kernel_mutations={kernel_mutations}, adapter_mutations={adapter_mutations}, "
    f"shell_initialization={shell_initialization}, unauthorized=0)"
)
