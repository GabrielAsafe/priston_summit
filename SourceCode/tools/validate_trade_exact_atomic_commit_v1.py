#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
C=ROOT/'Shared/gameplay/commerce/v2/TradeService.h'; A=ROOT/'SrcServer/src/systems/commerce/v2/legacy/LegacyTradeAdapter.cpp'; S=ROOT/'Shared/server/legacy_runtime/OnSever_legacy_server.inl'
def fail(m): print(f'trade exact atomic commit v1: FAIL: {m}',file=sys.stderr); raise SystemExit(1)
def fn(text):
 s=text.find('int\trsTradeInvenItem('); e=text.find('int SendChatMessage()',s)
 if s<0 or e<0: fail('cannot delimit trade function')
 return text[s:e]
def main():
 c=C.read_text(errors='replace'); a=A.read_text(errors='replace'); s=fn(S.read_text(errors='replace'))
 for tok in ('enum class TradeExactCommitCode','struct TradeExactCommitRequest','std::array<inventory::ItemInstanceId, kTradeOfferItemSlots> items','TradeExactCommitResult','virtual TradeExactCommitResult CommitExactTrade'):
  if tok not in c: fail(f'contract token missing: {tok}')
 for tok in ('LegacyTradeAdapter::CommitExactTrade','inventoryCommands_.Transfer(request.sender, request.receiver, request.items[index])','inventoryCommands_.Transfer(request.receiver, request.sender, request.items[slot])','CommitGold(TradeGoldCommitRequest','CommitCoins(TradeCoinCommitRequest','const auto rollbackGold','walletCommands_.Debit(request.receiver, request.gold, WalletReason::Trade)','walletCommands_.Credit(request.sender, request.gold, WalletReason::Trade)','TradeExactCommitCode::RollbackFailed'):
  if tok not in a: fail(f'atomic adapter token missing: {tok}')
 transfer=a.find('inventoryCommands_.Transfer(request.sender, request.receiver, request.items[index])')
 gold=a.find('CommitGold(TradeGoldCommitRequest',transfer)
 coin=a.find('CommitCoins(TradeCoinCommitRequest',gold)
 if not (0 <= transfer < gold < coin): fail('item -> gold -> coin commit ordering changed')
 for tok in ('bool exactTradeEligible = true','TradeExactCommitRequest exactTradeRequest','ServerLegacyTradeAdapter().CommitExactTrade(exactTradeRequest)','if(!exactCommit.Committed())','SaveThrowItem2(lpPlayInfo','MarkCommitted(tradeGuardRequest)'):
  if tok not in s: fail(f'handler exact path token missing: {tok}')
 exact_start=s.find('// Fully coordinated path for exact-item trades.')
 exact_end=s.find('\n\tif(lpTransTradeItemKey->Coin)',exact_start)
 exact=s[exact_start:exact_end]
 for bad in ('cCoinShop.AddPlayerCoin','CommitGold(', 'CommitCoins(', 'rsAddInvenItem(', 'rsDeleteInvenItem('):
  if bad in exact: fail(f'direct/partial authority leaked into exact path: {bad}')
 if 'sinPM1' not in exact: fail('potion exclusion guard missing')
 print('trade exact atomic commit v1: PASS (exact items + gold + coin coordinated with reverse item/gold compensation)')
if __name__=='__main__': main()
