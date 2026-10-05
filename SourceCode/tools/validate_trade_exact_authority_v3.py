#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
SERVER=ROOT/'Shared/server/legacy_runtime/OnSever_legacy_server.inl'
def fail(m): print(f'trade exact authority v3: FAIL: {m}',file=sys.stderr); raise SystemExit(1)
text=SERVER.read_text(errors='replace')
s=text.find('int\trsTradeInvenItem('); e=text.find('int SendChatMessage()',s)
if s<0 or e<0: fail('cannot delimit rsTradeInvenItem')
fn=text[s:e]
if fn.count('ServerLegacyTradeAdapter().CommitExactTrade(exactTradeRequest)') != 1:
    fail('rsTradeInvenItem must delegate exactly once to CommitExactTrade')
for token in ('if(!exactTradeEligible)','return FALSE;','MarkCommitted(tradeGuardRequest)','rsRecord_ItemLog_Trade(lpPlayInfo, lpPlayInfo2, lpTransTradeItemKey)'):
    if token not in fn: fail(f'missing exact-authority token: {token}')
for bad in ('ServerLegacyTradeAdapter().CommitGold(', 'ServerLegacyTradeAdapter().CommitCoins(', 'ServerLegacyTradeAdapter().TransferItem(', 'rsAddServerPotion(', 'rsAddInvenItem('):
    if bad in fn: fail(f'parallel legacy commit path remains: {bad}')
print('trade exact authority v3: PASS (one coordinated V2 commit path; invalid legacy fallback rejected)')
