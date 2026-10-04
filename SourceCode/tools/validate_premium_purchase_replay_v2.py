#!/usr/bin/env python3
from pathlib import Path

normal_client = Path('SrcGame/src/systems/commerce/coin_shop/CoinShop.cpp').read_text(errors='ignore')
time_client = Path('SrcGame/src/systems/commerce/coin_shop/CoinShop_T.cpp').read_text(errors='ignore')
normal_server = Path('SrcServer/src/systems/commerce/coin_shop/CoinShop.cpp').read_text(errors='ignore')
time_server = Path('SrcServer/src/systems/commerce/coin_shop/CoinShop_T.cpp').read_text(errors='ignore')
packet = Path('Shared/skillsPacket.h').read_text(errors='ignore')
checks = {
    'wire layout preserved': 'int ItemIndex;' in packet and 'struct smTRANS_BUYSHOP' in packet,
    'normal client emits request id': 'TransBuy.ItemIndex = static_cast<int>(SummitPremiumPurchaseRequestSequence().Next())' in normal_client,
    'time client emits request id': 'TransBuy.ItemIndex = static_cast<int>(SummitPremiumPurchaseRequestSequence().Next())' in time_client,
    'normal server reserves before purchase': 'SummitCoinShopReplayGuard().Reserve(operation)' in normal_server,
    'normal server commits only success': 'SummitCoinShopReplayGuard().Commit(operation)' in normal_server and 'SummitCoinShopReplayGuard().Abort(operation)' in normal_server,
    'time server reserves before purchase': 'SummitTimeShopReplayGuard().Reserve(operation)' in time_server,
    'time server commits only success': 'SummitTimeShopReplayGuard().Commit(operation)' in time_server and 'SummitTimeShopReplayGuard().Abort(operation)' in time_server,
    'account identity used': 'operation.purchaserKey = lpPlayInfo->getAccount()' in normal_server and 'operation.purchaserKey = lpPlayInfo->getAccount()' in time_server,
}
failed = [name for name, ok in checks.items() if not ok]
if failed:
    raise SystemExit('FAIL: ' + ', '.join(failed))
print('premium purchase replay v2: PASS')
