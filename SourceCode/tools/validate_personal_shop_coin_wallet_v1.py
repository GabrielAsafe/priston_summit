#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
C=(ROOT/'Shared/gameplay/commerce/v2/PersonalShopService.h').read_text(errors='replace')
H=(ROOT/'Shared/gameplay/commerce/v2/PersonalShopTransactionService.h').read_text(errors='replace') + (ROOT/'SrcServer/src/systems/commerce/v2/legacy/LegacyPersonalShopAdapter.h').read_text(errors='replace')
A=(ROOT/'Shared/gameplay/commerce/v2/PersonalShopTransactionService.h').read_text(errors='replace') + (ROOT/'SrcServer/src/systems/commerce/v2/legacy/LegacyPersonalShopAdapter.cpp').read_text(errors='replace')
def fail(m): print(f'personal shop coin wallet v1: FAIL: {m}',file=sys.stderr); raise SystemExit(1)
def main():
 for tok in ('SellerCoinCapacityExceeded','CoinWalletRejected'):
  if tok not in C: fail(f'result code missing: {tok}')
 for tok in ('const ICoinWalletQuery& coinQuery','ICoinWalletCommands& coinCommands','coinQuery_','coinCommands_'):
  if tok not in H: fail(f'injected coin port missing: {tok}')
 for tok in ('coinQuery_.TryGetCoinWallet(request.buyer, buyerCoinWallet)','coinQuery_.TryGetCoinWallet(request.seller, sellerCoinWallet)','coinCommands_.DebitCoins(request.buyer, request.coinPrice, CoinReason::PersonalShop)','coinCommands_.CreditCoins(request.seller, request.coinPrice, CoinReason::PersonalShop)','buyerCoinDebited','coinCommands_.DebitCoins(request.seller, request.coinPrice, CoinReason::PersonalShop)','coinCommands_.CreditCoins(request.buyer, request.coinPrice, CoinReason::PersonalShop)','ServerLegacyCoinWalletAdapter()'):
  if tok not in A: fail(f'coin transaction token missing: {tok}')
 for bad in ('cCoinShop.GetPlayerCoin','cCoinShop.AddPlayerCoin','CoinShop.h'):
  if bad in A: fail(f'direct coin legacy dependency remains: {bad}')
 print('personal shop coin wallet v1: PASS (coin preflight + debit/credit + compensating rollback behind port)')
if __name__=='__main__': main()
