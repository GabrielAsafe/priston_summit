#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
cpp=(root/'SrcServer/src/systems/commerce/vip/cVipHandler.cpp').read_text(errors='replace')
h=(root/'SrcServer/src/systems/commerce/vip/cVipHandler.h').read_text(errors='replace')
checks={
 'green entitlement service': (root/'Shared/gameplay/commerce/v2/PremiumEntitlementService.h').exists(),
 'amber vip repository': (root/'SrcServer/src/systems/commerce/v2/legacy/LegacyVipEntitlementRepository.cpp').exists(),
 'vip handler consumes service': 'ServerPremiumEntitlementService()' in cpp,
 'vip handler no direct db authority': 'SQLConnection::GetConnection' not in cpp,
 'shared mutable vip tier retired': 'int IdVip;' not in h,
 'transport preserved': 'SummitVipSend' in cpp and 'OPCODE_VIP' in cpp,
}
failed=[k for k,v in checks.items() if not v]
if failed:
 print('FAIL: '+', '.join(failed),file=sys.stderr);sys.exit(1)
print('vip entitlement authority v2 validator: PASS')
