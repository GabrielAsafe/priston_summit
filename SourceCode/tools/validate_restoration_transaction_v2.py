#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
cpp=(root/'SrcServer/src/systems/commerce/restoration/cRestaureHandler.cpp').read_text(errors='replace')
svc=(root/'Shared/gameplay/commerce/v2/RestorationTransactionService.h').read_text(errors='replace')
adapter=(root/'SrcServer/src/systems/commerce/restoration/v2/legacy/LegacyRestorationTransactionAdapters.h').read_text(errors='replace')
start=cpp.index('void cRESTAURE::RestaureItem')
end=cpp.index('void cRESTAURE::RecvGetRestaure',start)
body=cpp[start:end]
checks={
 'handler delegates transaction': 'RestorationTransactionService transaction' in body,
 'legacy inline coin debit removed': 'AddPlayerCoin(lpPlayInfo, -Price)' not in body,
 'legacy inline delete removed': 'DELETE FROM AgingFailed' not in body,
 'unsupported price rejected': 'RestorationPricingPolicy::UnsupportedPrice' in body,
 'service compensates consumed record': 'claimStore_.Restore(request.record)' in svc,
 'service compensates wallet': 'PremiumPurchaseRollback' in svc,
 'store consumes under transaction': 'BeginTransaction()' in adapter and 'CommitTransaction()' in adapter,
 'grant checks command result': 'CreateCommandItem(player_, player_, itemRef) == TRUE' in adapter,
}
failed=[n for n,v in checks.items() if not v]
if failed:
 for n in failed: print('FAIL:',n,file=sys.stderr)
 raise SystemExit(1)
print('restoration transaction v2: PASS')
