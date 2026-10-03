#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
contract=(ROOT/'Shared/gameplay/commerce/v2/PersonalShopSession.h').read_text('utf-8',errors='ignore')
service=(ROOT/'Shared/gameplay/commerce/v2/PersonalShopSessionService.h').read_text('utf-8',errors='ignore')
header=(ROOT/'SrcServer/src/systems/commerce/v2/legacy/LegacyPersonalShopAdapter.h').read_text('utf-8',errors='ignore')
adapter=(ROOT/'SrcServer/src/systems/commerce/v2/legacy/LegacyPersonalShopAdapter.cpp').read_text('utf-8',errors='ignore')
checks=[]
def ck(ok,label): checks.append((ok,label)); print(('PASS' if ok else 'FAIL')+': '+label)
ck('IPersonalShopSessionService' in contract,'GREEN personal shop session contract exists')
ck(all(x not in contract+service for x in ('rsPLAYINFO','sMYSHOP_SELL_INFO','MYSHOP_LIST_MAX','dwPlayServTime')),'GREEN session kernel is legacy-free')
ck('PlayerHandle buyer' in contract,'session identity is reconnect-generation aware')
ck('PruneStaleGeneration' in service,'new connection generation prunes stale session')
ck('IPersonalShopSessionService& sessionService' in header and 'sessionService_' in header,'legacy adapter consumes V2 session port')
prepare=adapter.split('PersonalShopPrepareResult LegacyPersonalShopAdapter::PrepareTrade',1)[1].split('PersonalShopMessageResult LegacyPersonalShopAdapter::ReadListingMessage',1)[0]
ck('sessionService_.CooldownActive' in prepare and 'sessionService_.RecordPrepared' in prepare,'prepare flow delegates cooldown/history to V2 session')
ck(all(x not in prepare for x in ('buyer->dwMyShopTradeTime','buyer->MyShopListCount','buyer->sMyShopList')),'prepare flow no longer owns legacy session fields')
ck('PersonalShopSessionService sessionService(MYSHOP_LIST_MAX, 500u)' in adapter,'adapter configures V2 kernel with preserved legacy limits')
if not all(x for x,_ in checks):
    print('personal shop session v2 authority: FAIL',file=sys.stderr); sys.exit(1)
print(f'personal shop session v2 authority: PASS ({len(checks)}/{len(checks)})')
