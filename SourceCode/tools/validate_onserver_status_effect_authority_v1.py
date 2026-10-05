#!/usr/bin/env python3
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
PATH = ROOT / "Shared/server/legacy_runtime/OnSever_legacy_server.inl"


def strip_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", lambda m: "\n" * m.group(0).count("\n"), text, flags=re.S)
    return re.sub(r"//[^\n]*", "", text)


def fail(message: str) -> None:
    print("onserver status effect authority v1: FAIL: " + message, file=sys.stderr)
    raise SystemExit(1)


text = PATH.read_text(encoding="utf-8", errors="replace")
active = strip_comments(text)

# OnServer may inspect legacy status fields, but gameplay application/clear decisions
# must cross the status authority boundary rather than mutate smCHAR state directly.
direct_write = re.compile(
    r"(?:PlayStunCount|PlaySlowCount|PlaySlowSpeed|PlayDistortion|"
    r"PlayPoison\s*\[[01]\]|PlayHolyMind\s*\[[01]\]|"
    r"PlayHeavenlyLight\s*\[[01]\])\s*(?:\+=|-=|=(?!=)|\+\+|--)"
)
hits = []
for line_no, line in enumerate(active.splitlines(), 1):
    if direct_write.search(line):
        hits.append((line_no, line.strip()))
if hits:
    for line_no, line in hits:
        print(f"FAIL: direct OnServer status mutation: {line_no}: {line}", file=sys.stderr)
    raise SystemExit(1)

required = {
    "LegacyStatusEffectAdapter include": '#include "systems/gameplay/combat/v2/legacy/LegacyStatusEffectAdapter.h"',
    "SoD stun routed": "ServerLegacyStatusEffectAdapter().ApplyStun(",
    "SoD slow routed": "ServerLegacyStatusEffectAdapter().ApplySlow(",
    "HolyMind routed": "ServerLegacyStatusEffectAdapter().ApplyHolyMind(",
    "stun clear routed": "ServerLegacyStatusEffectAdapter().ClearStatus(",
}
for label, token in required.items():
    if token not in active:
        fail(f"missing {label}")

if active.count("ServerLegacyStatusEffectAdapter().ApplyStun(") < 1:
    fail("expected at least one stun delegation")
if active.count("ServerLegacyStatusEffectAdapter().ApplySlow(") < 1:
    fail("expected at least one slow delegation")
if active.count("ServerLegacyStatusEffectAdapter().ApplyHolyMind(") < 1:
    fail("expected at least one HolyMind delegation")
if active.count("ServerLegacyStatusEffectAdapter().ClearStatus(") < 1:
    fail("expected at least one clear delegation")

print("onserver status effect authority v1: PASS (direct gameplay status writes=0)")
