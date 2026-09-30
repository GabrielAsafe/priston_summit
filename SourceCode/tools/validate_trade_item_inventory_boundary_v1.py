#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
CONTRACT = ROOT/'Shared/gameplay/commerce/v2/TradeService.h'
ADAPTER = ROOT/'SrcServer/src/systems/commerce/v2/legacy/LegacyTradeAdapter.cpp'
SERVER = ROOT/'Shared/server/legacy_runtime/OnSever_legacy_server.inl'

def fail(m):
    print(f'trade item inventory boundary v1: FAIL: {m}', file=sys.stderr); raise SystemExit(1)

def section(text):
    s=text.find('int\trsTradeInvenItem('); e=text.find('int SendChatMessage()', s)
    if s<0 or e<0: fail('cannot delimit rsTradeInvenItem')
    return text[s:e]

def main():
    c=CONTRACT.read_text(errors='replace'); a=ADAPTER.read_text(errors='replace'); s=section(SERVER.read_text(errors='replace'))
    for tok in ('struct TradeItemTransferRequest','inventory::InventoryTransferResult TransferItem','inventory::InventoryMutationResult RemoveItem'):
        if tok not in c: fail(f'contract token missing: {tok}')
    for bad in ('rsPLAYINFO','rsAddInvenItem','rsDeleteInvenItem'):
        if bad in c: fail(f'GREEN contract leaks legacy symbol: {bad}')
    for tok in ('inventoryCommands_.Transfer(request.sender, request.receiver, request.item)','inventoryCommands_.Remove(player, item)','ServerLegacyInventoryRuntime()'):
        if tok not in a: fail(f'adapter token missing: {tok}')
    for tok in ('ServerLegacyTradeAdapter().TransferItem(','TradeItemTransferRequest(','ServerLegacyTradeAdapter().RemoveItem(','duplicateRemoval.Applied()','SaveThrowItem2(lpPlayInfo'):
        if tok not in s: fail(f'trade handler token missing: {tok}')
    # Exact-item branch must no longer mutate inventory through the old functions.
    normal_start=s.find('if((lpTransTradeItemKey->dwItemCode[cnt] & sinITEM_MASK1) != (sinPM1&sinITEM_MASK1))', s.find('for(cnt = 0; cnt < MAX_TRADE_ITEM; cnt++)', s.find('CommitGold')))
    potion_else=s.find('\n\t\t\telse\n', normal_start)
    if normal_start < 0 or potion_else < 0: fail('cannot delimit normal item branch')
    normal=s[normal_start:potion_else]
    for bad in ('rsDeleteInvenItem(', 'rsAddInvenItem('):
        if bad in normal: fail(f'direct inventory mutation remains in normal item branch: {bad}')
    if 'rsAddServerPotion(' not in s: fail('potion stack path was unexpectedly removed')
    print('trade item inventory boundary v1: PASS (exact item transfer + replay cleanup behind inventory port; potion stack preserved)')

if __name__=='__main__': main()
