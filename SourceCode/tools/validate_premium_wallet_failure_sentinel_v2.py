#!/usr/bin/env python3
from pathlib import Path

checks = {
    'coin shop': Path('SrcServer/src/systems/commerce/coin_shop/CoinShop.cpp'),
    'time shop': Path('SrcServer/src/systems/commerce/coin_shop/CoinShop_T.cpp'),
}
for label, path in checks.items():
    text = path.read_text(errors='ignore')
    marker = 'int cCOINSHOP' if label == 'coin shop' else 'int cCOINSHOP_T'
    section = text[text.index(marker):]
    if 'int coin = -1;' not in section:
        raise SystemExit(f'{label}: missing deterministic failure sentinel')
print('premium wallet failure sentinel: PASS')
