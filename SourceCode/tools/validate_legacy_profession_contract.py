#!/usr/bin/env python3
from pathlib import Path
import re
import sys

root = Path(__file__).resolve().parents[1]
client = (root / 'SrcGame/src/systems/crafting/professions/JOBS.cpp').read_text(errors='replace')
reader = (root / 'SrcServer/src/systems/gameplay/progression/v2/LegacyProgressionReaders.cpp').read_text()

expected = {
    'arcanist': (1, 'ARCANISTA'),
    'blacksmith': (2, 'FERREIRO'),
    'lumberjack': (3, 'LENHADOR'),
    'miner': (4, 'MINERADOR'),
    'artisan': (5, 'ARTES'),
}

errors = []
for key, (code, label_prefix) in expected.items():
    mapping = rf'EqualsIgnoreCase\(key,\s*"{re.escape(key)}"\)\)\s*return\s+{code}\s*;'
    if not re.search(mapping, reader):
        errors.append(f'reader mapping missing: {key} -> {code}')
    ui_case = rf'case\s+{code}\s*:\s*.*?wsprintf\(nomejob,\s*"{label_prefix}'
    if not re.search(ui_case, client, flags=re.S):
        errors.append(f'legacy UI identity missing: JOB {code} -> {label_prefix}*')

# Selection pages prove the same five persisted JOB codes are actually offered.
selection_contract = {
    4: 1,  # arcanist
    5: 5,  # artisan
    6: 2,  # blacksmith
    7: 3,  # lumberjack
    8: 4,  # miner
}
for page, code in selection_contract.items():
    pattern = rf'case\s+{page}\s*:\s*SendJOBItemToServer\("",\s*{code}\s*,\s*0\s*\)\s*;'
    if not re.search(pattern, client, flags=re.S):
        errors.append(f'legacy profession selection drift: page {page} no longer selects JOB {code}')

for unsupported in ('fisher', 'hunter'):
    if re.search(rf'EqualsIgnoreCase\(key,\s*"{unsupported}"\)', reader, flags=re.I):
        errors.append(f'{unsupported} must remain unavailable until a canonical legacy JOB code exists')

if errors:
    for error in errors:
        print(f'legacy profession contract validation: FAIL: {error}', file=sys.stderr)
    sys.exit(1)

print('legacy profession contract validation: PASS (5 canonical legacy professions; fisher/hunter unavailable)')
