#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
C=ROOT/'Shared/gameplay/commerce/v2/TradeService.h'; A=ROOT/'SrcServer/src/systems/commerce/v2/legacy/LegacyTradeAdapter.cpp'; S=ROOT/'Shared/server/legacy_runtime/OnSever_legacy_server.inl'; O=ROOT/'Shared/legacy_oracle/server/core/server/legacy_runtime/OnSever.cpp.inl'
def fail(m): print(f'trade coin commit v1: FAIL: {m}',file=sys.stderr); raise SystemExit(1)
def fn(text):
 s=text.find('int\trsTradeInvenItem('); e=text.find('int SendChatMessage()',s)
 if s<0 or e<0: fail('cannot delimit trade function')
 return text[s:e]
def main():
 c=C.read_text(errors='replace'); a=A.read_text(errors='replace'); s=fn(S.read_text(errors='replace')); o=fn(O.read_text(errors='replace'))
 for tok in ('enum class TradeCoinCommitCode','struct TradeCoinCommitRequest','struct TradeCoinCommitResult','virtual TradeCoinCommitResult CommitCoins'):
  if tok not in c: fail(f'contract token missing: {tok}')
 for bad in ('cCoinShop','rsPLAYINFO'):
  if bad in c: fail(f'GREEN trade contract leaks legacy coin symbol: {bad}')
 for tok in ('coinQuery_.TryGetCoinWallet(request.sender, senderWallet)','coinQuery_.TryGetCoinWallet(request.receiver, receiverWallet)','coinCommands_.DebitCoins(request.sender, request.amount, CoinReason::Trade)','coinCommands_.CreditCoins(request.receiver, request.amount, CoinReason::Trade)','coinCommands_.CreditCoins(request.sender, request.amount, CoinReason::Trade)','TradeCoinCommitCode::RollbackFailed'):
  if tok not in a: fail(f'adapter token missing: {tok}')
 for tok in ('ServerLegacyTradeAdapter().CommitCoins(','TradeCoinCommitRequest(','if(!coinCommit.Committed())','coinCommit.senderBefore'):
  if tok not in s: fail(f'migrated handler token missing: {tok}')
 for bad in ('cCoinShop.GetPlayerCoin(lpPlayInfo)', 'cCoinShop.AddPlayerCoin(lpPlayInfo, -lpTransTradeItemKey->Coin)', 'cCoinShop.AddPlayerCoin(lpPlayInfo2, lpTransTradeItemKey->Coin)'):
  if bad in s: fail(f'direct coin authority remains in trade commit: {bad}')
 for tok in ('cCoinShop.GetPlayerCoin(lpPlayInfo) < lpTransTradeItemKey->Coin','cCoinShop.AddPlayerCoin(lpPlayInfo, -lpTransTradeItemKey->Coin)','cCoinShop.AddPlayerCoin(lpPlayInfo2, lpTransTradeItemKey->Coin)'):
  if tok not in o: fail(f'legacy oracle coin behavior missing: {tok}')
 print('trade coin commit v1: PASS (sender/receiver preflight + debit/credit + compensating sender rollback)')
if __name__=='__main__': main()
