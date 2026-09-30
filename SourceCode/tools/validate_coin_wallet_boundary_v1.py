#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
C=ROOT/'Shared/gameplay/commerce/v2/CoinWallet.h'; A=ROOT/'SrcServer/src/systems/commerce/v2/legacy/LegacyCoinWalletAdapter.cpp'
def fail(m): print(f'coin wallet boundary v1: FAIL: {m}',file=sys.stderr); raise SystemExit(1)
def main():
    c=C.read_text(errors='replace'); a=A.read_text(errors='replace')
    for tok in ('using CoinAmount = std::int64_t','class ICoinWalletQuery','class ICoinWalletCommands','CreditCoins','DebitCoins','InsufficientFunds','CapacityExceeded'):
        if tok not in c: fail(f'contract token missing: {tok}')
    for bad in ('rsPLAYINFO','cCoinShop','CoinShop.h'):
        if bad in c: fail(f'GREEN coin contract leaks legacy symbol: {bad}')
    for tok in ('cCoinShop.GetPlayerCoin(legacyPlayer)','cCoinShop.AddPlayerCoin(legacyPlayer, static_cast<int>(amount))','cCoinShop.AddPlayerCoin(legacyPlayer, -static_cast<int>(amount))','result.after == before + amount','result.after == before - amount'):
        if tok not in a: fail(f'adapter safety token missing: {tok}')
    print('coin wallet boundary v1: PASS (typed query/credit/debit + negative/overflow guards + postcondition verification)')
if __name__=='__main__': main()
