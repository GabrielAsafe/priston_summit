#!/usr/bin/env python3
from pathlib import Path
from collections import Counter
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
SERVER_ONSERVER = Path('Shared/server/legacy_runtime/OnSever_legacy_server.inl')
SCAN_ROOT = ROOT / 'SrcServer/src'

ADAPTERS = {
    Path('SrcServer/src/systems/gameplay/combat/v2/legacy/LegacyCombatBuffAdapter.cpp'),
    Path('SrcServer/src/systems/gameplay/combat/v2/legacy/LegacyDamageAdapter.cpp'),
    Path('SrcServer/src/systems/gameplay/combat/v2/legacy/LegacyLifeAdapter.cpp'),
    Path('SrcServer/src/systems/gameplay/combat/v2/legacy/LegacyHealingAdapter.cpp'),
    Path('SrcServer/src/systems/gameplay/inventory/v2/legacy/LegacyPotionAdapter.cpp'),
    Path('SrcServer/src/systems/gameplay/monsters/v2/legacy/LegacyMonsterAdapter.cpp'),
    Path('SrcServer/src/systems/gameplay/monsters/v2/legacy/LegacySummonedActorAdapter.cpp'),
}

# These are construction/hydration writes, not gameplay mutation authority. Counts are
# intentionally exact: adding another direct write requires an explicit gate review.
SHELL_BASELINE = {
    SERVER_ONSERVER: {
        'lpTower->smCharInfo.Life[0] = 30000;': 1,
        'lpHopy->smCharInfo.Life[0] = hp;': 1,
        'StgCharInfo.smCharInfo.Life[0] = 100;': 14,
        'lpChar->smCharInfo.Life[0] = lpChar->smCharInfo.Life[1];': 2,
        'stageInfo.smCharInfo.Life[0] = stageInfo.smCharInfo.Life[1];': 1,
        'lpChar->smCharInfo.Life[0] += (lpChar->smCharInfo.Life[0] * pow) / 100;': 1,
        'if(lpChar->smCharInfo.Life[0] < 0 || lpChar->smCharInfo.Life[0]>30000) lpChar->smCharInfo.Life[0] = 30000;': 1,
        'dst.smCharInfo.Life[0] = 100;': 2,
        'lpPlayInfo->smCharInfo.Life[0] = lpTransPlayPos->Hp[0];': 1,
        'lpPlayInfo->smCharInfo.Life[0] = lpTransPlayData->Hp[0];': 1,
        'lpChar->smCharInfo.Life[0] = Atalanta_T5_SummonTiger_Life[point];': 1,
        'lpChar->smCharInfo.Life[0] = Priestess_T5_IceElemental_Life[point];': 1,
        'lpChar->smCharInfo.Life[0] = Metal_Golem_Life[point] + lpPlayInfo->smCharInfo.Strength * 2;': 1,
        'lpChar->smCharInfo.Life[0] = Recall_Wolverin_Life[point];': 1,
        'lpChar->smCharInfo.Life[0] = Fire_Elemental_Life[point];': 1,
    },
    Path('SrcServer/src/systems/gameplay/events/server_legacy/svrEventSpawn.cpp'): {
        'StgCharInfo.smCharInfo.Life[0] = 100;': 2,
    },
    Path('SrcServer/src/systems/gameplay/events/server_legacy/SvrCaravan.cpp'): {
        'StgCharInfo.smCharInfo.Life[0] = 100;': 1,
    },
    Path('SrcServer/src/systems/gameplay/character/core/character.cpp'): {
        'smCharInfo.Life[0] = 100;': 1,
        'smCharInfo.Life[0] = 10;': 1,
        'smCharInfo.Life[0] = lpTransPlayData->Hp[0];': 2,
    },
    Path('SrcServer/src/core/server/gameplay_runtime/playmain.cpp'): {
        'lpCurPlayer->smCharInfo.Life[0] = 1000;': 1,
    },
}

WRITE = re.compile(r'(?:\b\w+->|\b\w+\.)?smCharInfo\.Life\[0\]\s*(?:\+=|-=|=(?!=)|\+\+|--)')


def strip_comments(text: str) -> str:
    text = re.sub(r'/\*.*?\*/', lambda m: '\n' * m.group(0).count('\n'), text, flags=re.S)
    return re.sub(r'//.*', '', text)


def mutations(path: Path):
    text = strip_comments((ROOT / path).read_text(encoding='utf-8', errors='ignore'))
    return [(idx, line.strip()) for idx, line in enumerate(text.splitlines(), 1) if WRITE.search(line)]

files = [SERVER_ONSERVER]
files.extend(
    p.relative_to(ROOT)
    for p in SCAN_ROOT.rglob('*')
    if p.is_file() and p.suffix.lower() in {'.cpp', '.h', '.inl', '.c'}
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

for rel, line_no, line in unauthorized:
    print(f'FAIL: unauthorized current-life mutation: {rel}:{line_no}: {line}')
for rel, line, expected, actual in baseline_errors:
    print(f'FAIL: shell life baseline changed: {rel}: expected {expected}, got {actual}: {line}')
if unauthorized or baseline_errors:
    sys.exit(1)
print(
    'server life mutation ownership v1: PASS '
    f'(adapter_mutations={adapter_mutations}, shell_state_writes={shell_mutations}, unauthorized=0)'
)
