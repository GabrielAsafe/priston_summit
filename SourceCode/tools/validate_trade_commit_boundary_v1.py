#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
CONTRACT=ROOT/'Shared/gameplay/commerce/v2/TradeService.h'
TX=ROOT/'Shared/gameplay/commerce/v2/TradeTransactionService.h'
SERVER=ROOT/'Shared/server/legacy_runtime/OnSever_legacy_server.inl'
ORACLE=ROOT/'Shared/legacy_oracle/server/core/server/legacy_runtime/OnSever.cpp.inl'
def fail(message): print(f'trade commit boundary v1: FAIL: {message}',file=sys.stderr); raise SystemExit(1)
def function(text,name,next_name):
 start=text.find(name); end=text.find(next_name,start)
 if start<0 or end<0: fail(f'cannot delimit {name}')
 return text[start:end]
def main():
 contract=CONTRACT.read_text(errors='replace'); tx=TX.read_text(errors='replace')
 server=function(SERVER.read_text(errors='replace'),'int\trsTradeInvenItem(','int SendChatMessage()')
 oracle=function(ORACLE.read_text(errors='replace'),'int\trsTradeInvenItem(','int SendChatMessage()')
 for tok in ('CurrencyAmount gold','GoldRejected','TradeExactCommitRequest','TradeExactCommitResult'):
  if tok not in contract: fail(f'exact trade gold contract missing: {tok}')
 for forbidden in ('rsPLAYINFO','ServerMoney','WHERE_TRADE','SubServerMoney','AddServerMoney'):
  if forbidden in contract: fail(f'GREEN trade contract leaks legacy symbol: {forbidden}')
 for tok in ('TransferGold(request.sender, request.receiver, request.gold)','walletQuery_.TryGetWallet(sender, senderWallet)','walletQuery_.TryGetWallet(receiver, receiverWallet)','walletCommands_.Debit(sender, amount, WalletReason::Trade)','walletCommands_.Credit(receiver, amount, WalletReason::Trade)','const auto rollbackGold','walletCommands_.Debit(request.receiver, request.gold, WalletReason::Trade)','walletCommands_.Credit(request.sender, request.gold, WalletReason::Trade)'):
  if tok not in tx: fail(f'exact transaction gold safety token missing: {tok}')
 for tok in ('const int decodedGold = lpTransTradeItemKey->Money - 193;','if(decodedGold < 0)','exactTradeRequest.gold = static_cast<gameplay::commerce::CurrencyAmount>(decodedGold);','ServerLegacyTradeAdapter().CommitExactTrade(exactTradeRequest)','RecordHackLogFile(lpPlayInfo, &smTransCommand);'):
  if tok not in server: fail(f'migrated exact trade token missing: {tok}')
 for forbidden in ('ServerLegacyTradeAdapter().CommitGold(', 'lpPlayInfo->SubServerMoney(', 'lpPlayInfo2->AddServerMoney('):
  if forbidden in server: fail(f'parallel/direct trade gold authority remains: {forbidden}')
 for tok in ('money = lpTransTradeItemKey->Money - 193;','if(lpPlayInfo->ServerMoney < money)','lpPlayInfo->SubServerMoney(money, WHERE_TRADE);','lpPlayInfo2->AddServerMoney(money, WHERE_TRADE);'):
  if tok not in oracle: fail(f'legacy oracle trade-gold behavior missing: {tok}')
 print('trade commit boundary v1: PASS (Money-193 decoded at ingress; gold settlement/rollback owned by exact transaction)')
if __name__=='__main__': main()
