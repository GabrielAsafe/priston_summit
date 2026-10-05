#!/usr/bin/env python3
from pathlib import Path
from collections import Counter
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
SERVER_ONSERVER = Path('Shared/server/legacy_runtime/OnSever_legacy_server.inl')
SCAN_ROOT = ROOT / 'SrcServer/src'

ADAPTERS = {
    Path('SrcServer/src/systems/gameplay/skills/v2/legacy/LegacySkillResourceAdapter.cpp'),
    Path('SrcServer/src/systems/gameplay/combat/v2/legacy/LegacyCombatBuffAdapter.cpp'),
    Path('SrcServer/src/systems/gameplay/inventory/v2/legacy/LegacyPotionAdapter.cpp'),
}
SHELL_BASELINE = {
    SERVER_ONSERVER: {
        'StgCharInfo.smCharInfo.Mana[0] = 100;': 14,
        'stageInfo.smCharInfo.Mana[0] = stageInfo.smCharInfo.Mana[1];': 1,
        'stageInfo.smCharInfo.Stamina[0] = stageInfo.smCharInfo.Stamina[1];': 1,
    },
    Path('SrcServer/src/systems/authentication/character_selection/HoLogin.cpp'): {
        'JobCharacter[JobCharacterCount - 1].smCharInfo.Stamina[0] = 100;': 1,
        'JobCharacter[index].smCharInfo.Stamina[0] = 100;': 1,
        'JobCharacter[JobBodySelectNum].smCharInfo.Stamina[0] = 1000;': 1,
    },
    Path('SrcServer/src/systems/gameplay/events/server_legacy/svrEventSpawn.cpp'): {
        'StgCharInfo.smCharInfo.Mana[0] = 100;': 2,
    },
    Path('SrcServer/src/systems/gameplay/events/server_legacy/SvrCaravan.cpp'): {
        'StgCharInfo.smCharInfo.Mana[0] = 100;': 1,
    },
}
WRITE = re.compile(r'(?:\b\w+->|\b\w+\.)?smCharInfo\.(?:Mana|Stamina)\[0\]\s*(?:\+=|-=|=(?!=)|\+\+|--)')

def strip_comments(text: str) -> str:
    text = re.sub(r'/\*.*?\*/', lambda m: '\n' * m.group(0).count('\n'), text, flags=re.S)
    return re.sub(r'//.*', '', text)

def mutations(rel: Path):
    text = strip_comments((ROOT / rel).read_text(encoding='utf-8', errors='ignore'))
    return [(i, line.strip()) for i, line in enumerate(text.splitlines(), 1) if WRITE.search(line)]

files=[SERVER_ONSERVER]
files += [p.relative_to(ROOT) for p in SCAN_ROOT.rglob('*') if p.is_file() and p.suffix.lower() in {'.cpp','.h','.inl','.c'}]
unauthorized=[]; adapter_mutations=0; shell_mutations=0; seen={}
for rel in files:
    hits=mutations(rel)
    if not hits: continue
    if rel in ADAPTERS:
        adapter_mutations += len(hits); continue
    allowed=SHELL_BASELINE.get(rel)
    if allowed is None:
        unauthorized += [(rel,n,line) for n,line in hits]; continue
    counts=Counter(line for _,line in hits); seen[rel]=counts
    for n,line in hits:
        if line in allowed: shell_mutations += 1
        else: unauthorized.append((rel,n,line))
errors=[]
for rel,expected in SHELL_BASELINE.items():
    actual=seen.get(rel,Counter())
    for line,count in expected.items():
        if actual[line] != count: errors.append((rel,line,count,actual[line]))
for rel,n,line in unauthorized:
    print(f'FAIL: unauthorized mana/stamina mutation: {rel}:{n}: {line}')
for rel,line,expected,actual in errors:
    print(f'FAIL: resource shell baseline changed: {rel}: expected {expected}, got {actual}: {line}')
if unauthorized or errors: sys.exit(1)
print(f'server resource mutation ownership v1: PASS (adapter_mutations={adapter_mutations}, shell_state_writes={shell_mutations}, unauthorized=0)')
