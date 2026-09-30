#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
contract=(ROOT/'Shared/gameplay/commerce/v2/ShopService.h').read_text(errors='replace')
adapter=(ROOT/'SrcServer/src/systems/commerce/v2/legacy/LegacyShopAdapter.cpp').read_text(errors='replace')
server=(ROOT/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='replace')
buy=server.split('int rsRecvBuyShopItem(',1)[1].split('int rsRecvSellShopItem(',1)[0]
def fail(m): print(f'npc shop atomic potion purchase v1: FAIL: {m}',file=sys.stderr); raise SystemExit(1)
def main():
 for tok in ('ShopPotionPurchaseRequest','ShopPotionPurchaseResult','ShopPotionPurchaseCode','PurchasePotion(const ShopPotionPurchaseRequest& request)','RollbackFailed'):
  if tok not in contract: fail(f'contract token missing: {tok}')
 for tok in ('potionStackQuery_.TryGetStack(','potionStackCommands_.AddStack(','walletCommands_.Debit(request.player, result.total, WalletReason::NpcBuyPotion)','potionStackCommands_.RemoveStack(','ShopPotionPurchaseCode::RollbackFailed'):
  if tok not in adapter: fail(f'atomic adapter token missing: {tok}')
 for tok in ('ServerLegacyShopAdapter().PurchasePotion(','ShopPotionPurchaseRequest(','if(isPotionPurchase)'):
  if tok not in buy: fail(f'server buy token missing: {tok}')
 if 'rsAddServerPotion(' in buy: fail('direct potion add remains in NPC shop handler')
 print('npc shop atomic potion purchase v1: PASS (stack grant + wallet debit + compensating stack rollback)')
if __name__=='__main__': main()
