#!/usr/bin/env python3
from pathlib import Path

root = Path(__file__).resolve().parents[5]
path = root / 'SrcServer/src/systems/commerce/legacy/sinTrade.cpp'
text = path.read_text(encoding='utf-8', errors='replace')
start = text.index('int cAGING::AgingItem(')
end = text.index('int cAGING::CancelAgingItem', start)
body = text[start:end]

if 'AgingStoneKindPolicy::Classify' not in body:
    raise SystemExit('legacy aging stone-kind guard: V2 policy not consumed')
for code in ('sinBI1 | sin10', 'sinBI1 | sin11', 'sinBI1 | sin60', 'sinBI1 | sin61', 'sinBI1 | sin85',
             'sinBI2 | sin73', 'sinBI2 | sin74', 'sinBI2 | sin75', 'sinBI2 | sin76'):
    if f'sAging_Recv.A_StoneItemCode == ({code})' in body:
        raise SystemExit(f'legacy aging stone-kind guard: inline stone branch remains: {code}')

print('legacy aging stone-kind V2 guard: PASS')
