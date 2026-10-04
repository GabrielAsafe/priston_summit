#!/usr/bin/env python3
from pathlib import Path

text = Path('SrcServer/src/systems/commerce/coin_shop/CoinShop_T.cpp').read_text(errors='ignore')
adapter = Path('SrcServer/src/systems/commerce/v2/legacy/LegacyTimeCreditWalletAdapter.h').read_text(errors='ignore')
section = text.split('void cCOINSHOP_T::RecvBuyShopItem', 1)[1].split('int cCOINSHOP_T::GetPlayerCoin', 1)[0]
checks = {
    'time shop uses premium transaction service': 'PremiumPurchaseTransactionService transaction' in section,
    'time shop no longer debits inline': 'UserMinutos=?' not in section and 'PlayerCoin -=' not in section,
    'time shop logs only purchased result': 'if(result.Purchased())' in section,
    'time wallet supports rollback credit': 'CreditCoins' in adapter and 'cCoinShop_T.AddPlayerCoin' in adapter,
    'time wallet rejects failed balance lookup': 'if(balance < 0)' in adapter,
}
failed = [name for name, ok in checks.items() if not ok]
if failed:
    raise SystemExit('FAIL: ' + ', '.join(failed))
print('time shop purchase transaction v2: PASS')
