#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
FILES = (
    ROOT / 'SrcServer/src/systems/commerce/coin_shop/CoinShop.cpp',
    ROOT / 'SrcServer/src/systems/commerce/coin_shop/CoinShop_T.cpp',
)

failed = []
for path in FILES:
    text = path.read_text(encoding='utf-8', errors='replace')
    label = path.name
    start = text.find('class Legacy')
    end = text.find('\n};', start)
    grant = text[start:end + 3] if start >= 0 and end >= 0 else ''
    checks = (
        ('inventory boundary include', 'systems/inventory/v2/legacy/LegacyInventoryRuntime.h' in text),
        ('grant uses inventory command', 'ServerLegacyInventoryRuntime().Add(' in grant),
        ('grant uses exact item identity', 'inventory::ItemInstanceId(' in grant),
        ('grant requires applied mutation', 'InventoryMutationCode::Applied' in grant),
        ('no direct rsAddInvenItem grant', 'rsAddInvenItem(' not in grant),
    )
    for name, ok in checks:
        print(('PASS' if ok else 'FAIL') + f': {label}: {name}')
        if not ok:
            failed.append(f'{label}: {name}')

if failed:
    for item in failed:
        print('premium inventory grant boundary v3: FAIL: ' + item, file=sys.stderr)
    raise SystemExit(1)
print('premium inventory grant boundary v3: PASS (coin/time premium grants use inventory command boundary)')
