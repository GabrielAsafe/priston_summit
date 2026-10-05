#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
CONTRACT=ROOT/'Shared/gameplay/commerce/v2/TradeService.h'
TX=ROOT/'Shared/gameplay/commerce/v2/TradeTransactionService.h'
SERVER=ROOT/'Shared/server/legacy_runtime/OnSever_legacy_server.inl'
def fail(m): print(f'trade item inventory boundary v1: FAIL: {m}',file=sys.stderr); raise SystemExit(1)
def section(text):
 s=text.find('int\trsTradeInvenItem('); e=text.find('int SendChatMessage()',s)
 if s<0 or e<0: fail('cannot delimit rsTradeInvenItem')
 return text[s:e]
def main():
 c=CONTRACT.read_text(errors='replace'); tx=TX.read_text(errors='replace'); s=section(SERVER.read_text(errors='replace'))
 for tok in ('std::array<inventory::ItemInstanceId, kTradeOfferItemSlots> items','std::array<TradePotionStackOffer, kTradeOfferItemSlots> potionStacks'):
  if tok not in c: fail(f'exact item contract token missing: {tok}')
 for bad in ('rsPLAYINFO','rsAddInvenItem','rsDeleteInvenItem'):
  if bad in c: fail(f'GREEN contract leaks legacy symbol: {bad}')
 for tok in ('inventoryReservations_.Reserve(reservationRequest)','inventoryCommands_.Transfer(request.sender, request.receiver, request.items[index])','inventoryCommands_.Transfer(request.receiver, request.sender, request.items[slot])','potionStackCommands_.TransferStack('):
  if tok not in tx: fail(f'exact transaction inventory token missing: {tok}')
 for tok in ('exactTradeRequest.items[cnt] = gameplay::inventory::ItemInstanceId(','exactTradeRequest.potionStacks[cnt] = gameplay::commerce::TradePotionStackOffer(','ServerLegacyTradeAdapter().CommitExactTrade(exactTradeRequest)','ServerLegacyTradeAdapter().RemoveItem(','duplicateRemoval.Applied()','SaveThrowItem2(lpPlayInfo'):
  if tok not in s: fail(f'trade handler token missing: {tok}')
 for bad in ('ServerLegacyTradeAdapter().TransferItem(', 'rsAddServerPotion(', 'rsAddInvenItem('):
  if bad in s: fail(f'parallel/direct inventory authority remains in rsTradeInvenItem: {bad}')
 print('trade item inventory boundary v1: PASS (exact item/potion transfer owned by reserved transaction; replay cleanup remains adapter-owned)')
if __name__=='__main__': main()
