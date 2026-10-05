#!/usr/bin/env python3
from collections import Counter
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
SCAN_ROOT = ROOT / "SrcServer/src"
ONSERVER = Path("Shared/server/legacy_runtime/OnSever_legacy_server.inl")
CHARACTER = Path("SrcServer/src/systems/gameplay/character/core/character.cpp")
ADAPTERS = {
    Path("SrcServer/src/systems/gameplay/combat/v2/legacy/LegacyStatusEffectAdapter.cpp"),
}

# Construction/reset, timer ticking and packet-driven presentation state are still
# legacy runtime shell responsibilities. These exact writes are not feature authority.
SHELL_BASELINE = {
    CHARACTER: Counter({
        "PlaySlowCount = 0;": 1,
        "PlaySlowSpeed = 0;": 1,
        "PlayStunCount = 0;": 1,
        "PlayHolyMind[1] = 0;": 1,
        "PlayHeavenlyLight[0] = 0;": 1,
        "PlayHeavenlyLight[1] = 0;": 1,
        "PlayPoison[1] = 0;": 2,
        "PlayDistortion = 0;": 2,
        "dwAssassinEyeTime = 0;": 1,
        "AssassinEyeParam = 0;": 1,
        "PlaySlowCount--;": 2,
        "if(PlayDistortion > 0) PlayDistortion--;": 1,
        "PlayStunCount--;": 2,
        "PlayPoison[1]--;": 2,
        "PlaySlowCount++;": 1,
        "PlayHolyMind[1]--;": 1,
        "PlayHeavenlyLight[1]--;": 1,
        "PlayDistortion--;": 1,
        "PlayDistortion = 120;": 1,
    }),
}

WRITE = re.compile(
    r"(?:PlayStunCount|PlaySlowCount|PlaySlowSpeed|PlayDistortion|"
    r"PlayPoison\s*\[[01]\]|PlayHolyMind\s*\[[01]\]|"
    r"dwAssassinEyeTime|AssassinEyeParam|PlayHeavenlyLight\s*\[[01]\])"
    r"\s*(?:\+=|-=|=(?!=)|\+\+|--)"
)


def strip_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", lambda m: "\n" * m.group(0).count("\n"), text, flags=re.S)
    return re.sub(r"//[^\n]*", "", text)


def mutations(rel: Path):
    text = strip_comments((ROOT / rel).read_text(encoding="utf-8", errors="ignore"))
    return [(i, line.strip()) for i, line in enumerate(text.splitlines(), 1) if WRITE.search(line)]


files = [ONSERVER]
files.extend(
    p.relative_to(ROOT)
    for p in SCAN_ROOT.rglob("*")
    if p.is_file() and p.suffix.lower() in {".cpp", ".h", ".inl", ".c"}
)

unauthorized = []
adapter_mutations = 0
shell_mutations = 0
seen_shell = {}
for rel in files:
    hits = mutations(rel)
    if not hits:
        continue
    if rel in ADAPTERS:
        adapter_mutations += len(hits)
        continue
    allowed = SHELL_BASELINE.get(rel)
    if allowed is None:
        unauthorized.extend((rel, line_no, line) for line_no, line in hits)
        continue
    counts = Counter(line for _, line in hits)
    seen_shell[rel] = counts
    for line_no, line in hits:
        if line not in allowed:
            unauthorized.append((rel, line_no, line))
        else:
            shell_mutations += 1

baseline_errors = []
for rel, expected in SHELL_BASELINE.items():
    actual = seen_shell.get(rel, Counter())
    for line, expected_count in expected.items():
        if actual[line] != expected_count:
            baseline_errors.append((rel, line, expected_count, actual[line]))
    for line, actual_count in actual.items():
        if line not in expected:
            baseline_errors.append((rel, line, 0, actual_count))

for rel, line_no, line in unauthorized:
    print(f"FAIL: unauthorized status mutation: {rel}:{line_no}: {line}", file=sys.stderr)
for rel, line, expected, actual in baseline_errors:
    print(
        f"FAIL: status shell baseline changed: {rel}: expected {expected}, got {actual}: {line}",
        file=sys.stderr,
    )
if unauthorized or baseline_errors:
    raise SystemExit(1)

print(
    "server status effect mutation ownership v1: PASS "
    f"(adapter_mutations={adapter_mutations}, shell_state_writes={shell_mutations}, unauthorized=0)"
)
