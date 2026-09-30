#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[1]
inl=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text()
adapter=(root/'SrcServer/src/systems/commerce/v2/legacy/LegacyPersonalShopAdapter.cpp').read_text()
contract=(root/'Shared/gameplay/commerce/v2/PersonalShopService.h').read_text()
anchor='int\trsTrade_PersonalShop(rsPLAYINFO* lpPlayInfo, rsPLAYINFO* lpPlayInfo2, smTRANS_COMMAND_BUFF* lpTransCommandBuff)'
fn=inl.split(anchor,1)[1].split('//Vl',1)[0]
case2=fn.split('case 2:',1)[1].split('return gameplay::commerce::legacy::ServerLegacyPersonalShopAdapter().CommitTrade(request).Committed() ? TRUE : FALSE;',1)[0] + 'return gameplay::commerce::legacy::ServerLegacyPersonalShopAdapter().CommitTrade(request).Committed() ? TRUE : FALSE;'
checks={
 'typed trade request':'struct PersonalShopTradeRequest' in contract,
 'typed commit result':'PersonalShopTradeResult' in contract and 'CommitTrade' in contract,
 'legacy case delegates':'ServerLegacyPersonalShopAdapter().CommitTrade(request)' in case2,
 'no direct wallet mutation in case2':'SubServerMoney' not in case2 and 'AddServerMoney' not in case2,
 'no direct coin mutation in case2':'AddPlayerCoin' not in case2,
 'no direct item mutation in case2':'rsDeleteInvenItem' not in case2 and 'rsAddInvenItem' not in case2,
 'adapter uses wallet boundary':'walletCommands_.Debit' in adapter and 'walletCommands_.Credit' in adapter,
 'adapter uses coin wallet boundary':'coinCommands_.DebitCoins' in adapter and 'coinCommands_.CreditCoins' in adapter and 'cCoinShop.AddPlayerCoin' not in adapter,
 'adapter uses inventory boundary':'inventoryCommands_.Remove' in adapter and 'inventoryCommands_.Add' in adapter and 'inventoryQuery_.Contains' in adapter,
 'adapter no direct exact-item mutation':'rsDeleteInvenItem' not in adapter and 'rsAddInvenItem' not in adapter and 'rsFindInvenItem' not in adapter,
 'listing guard preserved':'seller->sMyShopList[cnt]' in adapter,
 'potion path preserved':'potionStackCommands_.TransferStack' in adapter and 'potionStackQuery_.TryGetStack' in adapter and 'rsServerPotionErrorLog' in adapter,
 'item log preserved':'rsRecord_ItemLog_MyShop' in adapter,
}
for k,v in checks.items(): print(('PASS' if v else 'FAIL')+': '+k)
if not all(checks.values()): raise SystemExit(1)
print('personal shop transaction boundary v1: PASS')
