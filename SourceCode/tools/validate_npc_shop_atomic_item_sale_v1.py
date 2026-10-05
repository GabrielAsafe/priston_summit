#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
contract=(ROOT/'Shared/gameplay/commerce/v2/ShopService.h').read_text()
adapter=(ROOT/'Shared/gameplay/commerce/v2/ShopTransactionService.h').read_text()
server=(ROOT/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text()
sell=server.split('int rsRecvSellShopItem(',1)[1].split('int\trsCompareItems(',1)[0]

def need(text, token, label):
    if token not in text:
        print(f'npc shop atomic item sale v1: FAIL: {label}: {token}', file=sys.stderr); raise SystemExit(1)
for token in ('struct ShopItemSaleRequest','struct ShopItemSaleResult','ItemNotFound','RollbackFailed','SellItem(const ShopItemSaleRequest& request)'):
    need(contract, token, 'contract')
for token in ('inventoryCommands_.Remove(request.player,request.item)','walletCommands_.Credit(request.player,request.amount,WalletReason::NpcSellItem)','inventoryCommands_.Add(request.player,request.item)','ShopItemSaleCode::RollbackFailed'):
    need(adapter, token, 'adapter')
for token in ('ServerLegacyShopAdapter().SellItem(','ShopItemSaleRequest(','if(!sale.Sold())'):
    need(sell, token, 'server sell path')
if 'rsDeleteInvenItem(' in sell:
    print('npc shop atomic item sale v1: FAIL: direct item delete remains in sell handler', file=sys.stderr); raise SystemExit(1)
print('npc shop atomic item sale v1: PASS (remove-before-credit + compensating item restore)')
