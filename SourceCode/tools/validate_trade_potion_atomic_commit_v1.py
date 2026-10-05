#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
C=(ROOT/'Shared/gameplay/commerce/v2/TradeService.h').read_text(errors='replace')
H=(ROOT/'SrcServer/src/systems/commerce/v2/legacy/LegacyTradeAdapter.h').read_text(errors='replace')
A=(ROOT/'Shared/gameplay/commerce/v2/TradeTransactionService.h').read_text(errors='replace') + (ROOT/'SrcServer/src/systems/commerce/v2/legacy/LegacyTradeAdapter.cpp').read_text(errors='replace')
S=(ROOT/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='replace')
def fail(m): print(f'trade potion atomic commit v1: FAIL: {m}',file=sys.stderr); raise SystemExit(1)
def fn(t):
 s=t.find('int\trsTradeInvenItem('); e=t.find('int SendChatMessage()',s)
 if s<0 or e<0: fail('cannot delimit trade function')
 return t[s:e]
def main():
 s=fn(S)
 for tok in ('struct TradePotionStackOffer','std::array<TradePotionStackOffer, kTradeOfferItemSlots> potionStacks','PotionTransferRejected'):
  if tok not in C: fail(f'contract token missing: {tok}')
 for tok in ('inventory::IPotionStackCommands& potionStackCommands','potionStackCommands_'):
  if tok not in H: fail(f'adapter injection missing: {tok}')
 for tok in ('potionStackCommands_.TransferStack(','request.receiver, request.sender, offer.potion, offer.quantity','potionTransferred[slot]','TradeExactCommitCode::PotionTransferRejected','ServerLegacyPotionStackAdapter()'):
  if tok not in A: fail(f'atomic potion adapter token missing: {tok}')
 for tok in ('encodedPotionCount = 0x5cbc','encodedPotionCount &= 0xFFFF','exactTradeRequest.potionStacks[cnt]','TradePotionStackOffer(','rsAddTradePotionInfo(lpPlayInfo'):
  if tok not in s: fail(f'handler potion token missing: {tok}')
 exact=s[s.find('// Fully coordinated path for exact-item trades.'):s.find('\n\tif(lpTransTradeItemKey->Coin)',s.find('// Fully coordinated path for exact-item trades.'))]
 for bad in ('rsAddServerPotion(', 'rsGetServerPotion('):
  if bad in exact: fail(f'direct potion mutation leaked into coordinated path: {bad}')
 print('trade potion atomic commit v1: PASS (legacy quantity decode + stack transfer + reverse compensation inside coordinated commit)')
if __name__=='__main__': main()
