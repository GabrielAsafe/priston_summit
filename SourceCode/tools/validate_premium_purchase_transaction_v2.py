#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[1]; cpp=(root/'SrcServer/src/systems/commerce/coin_shop/CoinShop.cpp').read_text(errors='replace')
region=cpp.split('void cCOINSHOP::RecvBuyShopItem',1)[1].split('int cCOINSHOP::GetPlayerCoin',1)[0]
for req in ('PremiumPurchaseTransactionService','transaction.Purchase(request)','LegacyCoinShopGrantAdapter'):
    if req not in cpp: raise SystemExit('missing premium transaction path: '+req)
if 'UPDATE UserInfo SET UserCoin=' in region or 'PlayerCoin -=' in region:
    raise SystemExit('CoinShop handler still owns direct debit mutation')
service=(root/'Shared/gameplay/commerce/v2/PremiumPurchaseTransactionService.h').read_text(errors='replace')
if 'PremiumPurchaseRollback' not in service or 'RollbackFailed' not in service:
    raise SystemExit('premium transaction lacks compensating rollback')
print('premium purchase transaction validator: PASS')
