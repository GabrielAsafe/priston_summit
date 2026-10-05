#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
C=ROOT/'Shared/gameplay/commerce/v2/TradeService.h'
TX=ROOT/'Shared/gameplay/commerce/v2/TradeTransactionService.h'
S=ROOT/'Shared/server/legacy_runtime/OnSever_legacy_server.inl'
O=ROOT/'Shared/legacy_oracle/server/core/server/legacy_runtime/OnSever.cpp.inl'
def fail(m): print(f'trade coin commit v1: FAIL: {m}',file=sys.stderr); raise SystemExit(1)
def fn(text):
 s=text.find('int\trsTradeInvenItem('); e=text.find('int SendChatMessage()',s)
 if s<0 or e<0: fail('cannot delimit trade function')
 return text[s:e]
def main():
 c=C.read_text(errors='replace'); tx=TX.read_text(errors='replace'); s=fn(S.read_text(errors='replace')); o=fn(O.read_text(errors='replace'))
 for tok in ('CoinAmount coins','CoinRejected','TradeExactCommitRequest','TradeExactCommitResult'):
  if tok not in c: fail(f'exact trade coin contract token missing: {tok}')
 for bad in ('cCoinShop','rsPLAYINFO'):
  if bad in c: fail(f'GREEN trade contract leaks legacy coin symbol: {bad}')
 for tok in ('TransferCoins(request.sender, request.receiver, request.coins)','coinQuery_.TryGetCoinWallet(sender, senderWallet)','coinQuery_.TryGetCoinWallet(receiver, receiverWallet)','coinCommands_.DebitCoins(sender, amount, CoinReason::Trade)','coinCommands_.CreditCoins(receiver, amount, CoinReason::Trade)','coinCommands_.CreditCoins(sender, amount, CoinReason::Trade)'):
  if tok not in tx: fail(f'exact transaction coin safety token missing: {tok}')
 for tok in ('exactTradeRequest.coins = static_cast<gameplay::commerce::CoinAmount>(lpTransTradeItemKey->Coin);','ServerLegacyTradeAdapter().CommitExactTrade(exactTradeRequest)','TradeExactCommitCode::CoinRejected'):
  if tok not in s: fail(f'migrated exact handler token missing: {tok}')
 for bad in ('ServerLegacyTradeAdapter().CommitCoins(', 'cCoinShop.GetPlayerCoin(lpPlayInfo)', 'cCoinShop.AddPlayerCoin(lpPlayInfo, -lpTransTradeItemKey->Coin)', 'cCoinShop.AddPlayerCoin(lpPlayInfo2, lpTransTradeItemKey->Coin)'):
  if bad in s: fail(f'parallel/direct coin authority remains in trade commit: {bad}')
 for tok in ('cCoinShop.GetPlayerCoin(lpPlayInfo) < lpTransTradeItemKey->Coin','cCoinShop.AddPlayerCoin(lpPlayInfo, -lpTransTradeItemKey->Coin)','cCoinShop.AddPlayerCoin(lpPlayInfo2, lpTransTradeItemKey->Coin)'):
  if tok not in o: fail(f'legacy oracle coin behavior missing: {tok}')
 print('trade coin commit v1: PASS (coin settlement owned by exact transaction with compensating rollback)')
if __name__=='__main__': main()
