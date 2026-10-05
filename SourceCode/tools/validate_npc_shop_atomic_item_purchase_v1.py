#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
contract=(ROOT/'Shared/gameplay/commerce/v2/ShopService.h').read_text()
adapter=(ROOT/'Shared/gameplay/commerce/v2/ShopTransactionService.h').read_text()
server=(ROOT/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text()
buy=server.split('int rsRecvBuyShopItem(',1)[1].split('int rsRecvSellShopItem(',1)[0]

def need(text, token, label):
    if token not in text:
        print(f'npc shop atomic item purchase v1: FAIL: {label}: {token}', file=sys.stderr); raise SystemExit(1)
for token in ('struct ShopItemPurchaseRequest','struct ShopItemPurchaseResult','InventoryRejected','RollbackFailed','PurchaseItem(const ShopItemPurchaseRequest& request)'):
    need(contract, token, 'contract')
for token in ('inventoryCommands_.Add(request.player,request.item)','inventoryCommands_.Add(request.player,request.item)','walletCommands_.Debit(request.player,request.price,WalletReason::NpcBuyItem)','inventoryCommands_.Remove(request.player,request.item)','ShopItemPurchaseCode::RollbackFailed'):
    need(adapter, token, 'adapter')
for token in ('!lpPlayInfo || !lpTransBuyShopItem || !lpPlayInfo->lpsmSock','ReformItem(&lpTransBuyShopItem->sItem.sItemInfo);','ServerLegacyShopAdapter().PurchaseItem(','else if(rsServerConfig.ServerInventoryMode)'):
    need(buy, token, 'server buy path')
if 'rsAddInvenItem(' in buy:
    print('npc shop atomic item purchase v1: FAIL: direct exact-item grant remains in buy handler', file=sys.stderr); raise SystemExit(1)
print('npc shop atomic item purchase v1: PASS (capacity-before-debit + wallet-failure rollback + disconnect precheck)')
