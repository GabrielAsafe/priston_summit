#!/usr/bin/env python3
from pathlib import Path
import json, sys
ROOT = Path(__file__).resolve().parents[1]
base = json.loads((ROOT/'tools/architecture/direct_socket_send_baseline.json').read_text())
files = base.get('files', base)
checks = [
    ('SrcServer/src/systems/commerce/coin_shop/CoinShop.cpp', 'SummitCoinShopSend', 4),
    ('SrcServer/src/systems/commerce/coin_shop/CoinShop_T.cpp', 'SummitCoinShopTSend', 4),
]
fail=[]
for rel, helper, calls in checks:
    text=(ROOT/rel).read_text(errors='replace')
    if 'lpsmSock->Send' in text:
        fail.append(f'{rel}: direct socket send remains')
    if text.count(helper+'(') != calls + 1:
        fail.append(f'{rel}: expected helper + {calls} calls, found {text.count(helper+"(")}')
    if 'ServerLegacyPacketSender().Send(' not in text:
        fail.append(f'{rel}: helper is not backed by IPacketSender')
    if files.get(rel,{}).get('max_counts',{}).get('lpsmSock') != 0:
        fail.append(f'{rel}: lpsmSock baseline is not zero')
    for token in ('OPCODE_GET_SHOPITEM','OPCODE_GET_SHOPCOIN','OPCODE_PUTITEM'):
        if token not in text:
            fail.append(f'{rel}: preserved opcode missing: {token}')
if fail:
    for x in fail: print('FAIL:',x,file=sys.stderr)
    sys.exit(1)
print('coin shop packet sender v2: PASS (8/8 direct sends delegated, baselines=0)')
